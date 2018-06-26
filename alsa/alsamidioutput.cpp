/*
    KMid Backend using the ALSA Sequencer
    Copyright (C) 2009-2010 Pedro Lopez-Cabanillas <plcl@users.sf.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "alsamidioutput.h"
#include "midimapper.h"

#include <cmath>
#include <alsaclient.h>
#include <alsaport.h>
#include <alsaevent.h>

#include <QString>
#include <QStringList>
#include <QMutex>
#include <QMutexLocker>

using namespace drumstick;

namespace KMid {

    class ALSAMIDIOutput::ALSAMIDIOutputPrivate {
    public:
        ALSAMIDIOutputPrivate(ALSAMIDIOutput *q) :
            m_out(q),
            m_client(0),
            m_port(0),
            m_mapper(0),
            m_portId(0),
            m_pitchShift(0),
            m_clientFilter(true),
            m_runtimeAlsaDrivers(0)
        {
            for (int chan = 0; chan < MIDI_CHANNELS; ++chan) {
                m_lastpgm[chan] = 0;
                m_volumeShift[chan] = 1.0;
                m_volume[chan] = 100;
                m_muted[chan] = false;
                m_locked[chan] = false;
                m_lockedpgm[chan] = 0;
            }
            m_runtimeAlsaDrivers = getRuntimeALSADriverNumber();
        }

        ALSAMIDIOutput *m_out;
        MidiClient *m_client;
        MidiPort *m_port;
        MidiMapper *m_mapper;
        int m_portId;
        int m_pitchShift;
        bool m_clientFilter;
        int m_runtimeAlsaDrivers;
        QString m_currentOutput;
        QStringList m_outputDevices;
        int m_lastpgm[MIDI_CHANNELS];
        int m_lockedpgm[MIDI_CHANNELS];
        qreal m_volumeShift[MIDI_CHANNELS];
        int m_volume[MIDI_CHANNELS];
        bool m_muted[MIDI_CHANNELS];
        bool m_locked[MIDI_CHANNELS];
        QByteArray m_resetMessage;
        QMutex m_outMutex;

        void transformControllerEvent(SequencerEvent *ev)
        {
            ControllerEvent *event = static_cast<ControllerEvent*>(ev);
            if (m_mapper != NULL && m_mapper->isOK()) {
                int param = event->getParam();
                param = m_mapper->controller(param);
                if (param >= 0 && param < 128)
                    event->setParam(param);
            }
            if (event->getParam() == MIDI_CTL_MSB_MAIN_VOLUME) {
                int chan = event->getChannel();
                int value = event->getValue();
                m_volume[chan] = value;
                value = floor(value * m_volumeShift[chan]);
                if (value < 0) value = 0;
                if (value > 127) value = 127;
                event->setValue(value);
            }
        }

        void transformNoteEvent(SequencerEvent *ev)
        {
            int note, channel;
            NoteEvent *event = static_cast<NoteEvent*>(ev);
            channel = event->getChannel();
            if (channel != MIDI_GM_DRUM_CHANNEL) {
                note = event->getKey() + m_pitchShift;
                while (note > 127) note -= 12;
                while (note < 0) note += 12;
                event->setKey(note);
            } else if (m_mapper != NULL && m_mapper->isOK()) {
                note = m_mapper->key( event->getChannel(),
                                      m_lastpgm[channel],
                                      event->getKey() );
                if (note >= 0 && note < 128)
                    event->setKey(note);
            }
        }

        void transformProgramEvent(SequencerEvent *ev)
        {
            ProgramChangeEvent *event = static_cast<ProgramChangeEvent*>(ev);
            int channel = event->getChannel();
            m_lastpgm[channel] = event->getValue();
            if (m_mapper != NULL && m_mapper->isOK()) {
                int pgm = m_mapper->patch(channel, m_lastpgm[channel]);
                if (pgm >= 0 && pgm < 128)
                    event->setValue(pgm);
            }
        }

        void transformPitchBendEvent(SequencerEvent *ev)
        {
            PitchBendEvent *event = static_cast<PitchBendEvent*>(ev);
            if (m_mapper != NULL && m_mapper->isOK()) {
                int value = event->getValue();
                value = m_mapper->pitchBender(value);
                if (value < -8192) value = -8192;
                if (value > 8191) value = 8191;
                event->setValue(value);
            }
        }

        void transformEvent(SequencerEvent *ev)
        {
            switch ( ev->getSequencerType() ) {
            case SND_SEQ_EVENT_CONTROLLER:
                transformControllerEvent(ev);
                break;
            case SND_SEQ_EVENT_NOTEOFF:
            case SND_SEQ_EVENT_NOTEON:
                transformNoteEvent(ev);
                break;
            case SND_SEQ_EVENT_PGMCHANGE:
                transformProgramEvent(ev);
                break;
            case SND_SEQ_EVENT_PITCHBEND:
                transformPitchBendEvent(ev);
                break;
            default:
                break;
            }
            if (SequencerEvent::isChannel(ev) &&
                m_mapper != NULL && m_mapper->isOK()) {
                ChannelEvent *event = static_cast<ChannelEvent*>(ev);
                int channel = m_mapper->channel(event->getChannel());
                if (channel >= 0 && channel < MIDI_CHANNELS)
                    event->setChannel(channel);
            }
        }

        bool clientIsAdvanced(int clientId)
        {
            // asking for runtime drivers version instead of SND_LIB_VERSION
            if (m_runtimeAlsaDrivers < 0x01000B)
                // ALSA <= 1.0.10
                return (clientId < 64);
            else
                // ALSA >= 1.0.11
                return (clientId < 16);
        }
    };

    ALSAMIDIOutput::ALSAMIDIOutput(QObject *parent) : MIDIOutput(parent),
        d(new ALSAMIDIOutputPrivate(this))
    {
        d->m_client = new MidiClient(this);
        d->m_client->open();
        d->m_client->setClientName("KMid");
        d->m_port = d->m_client->createPort();
        d->m_port->setPortName("KMid");
        d->m_port->setCapability( SND_SEQ_PORT_CAP_READ  |
                                  SND_SEQ_PORT_CAP_SUBS_READ |
                                  SND_SEQ_PORT_CAP_WRITE );
        d->m_port->setPortType( SND_SEQ_PORT_TYPE_APPLICATION |
                                SND_SEQ_PORT_TYPE_MIDI_GENERIC );
        d->m_portId = d->m_port->getPortId();
        reloadDeviceList();
    }

    ALSAMIDIOutput::~ALSAMIDIOutput()
    {
        delete d;
    }

    qreal ALSAMIDIOutput::volume(int channel) const
    {
        if (channel >=0 && channel < MIDI_CHANNELS)
            return d->m_volumeShift[channel];
        return -1.0;
    }

    int ALSAMIDIOutput::outputDevice() const
    {
        return d->m_outputDevices.indexOf(d->m_currentOutput);
    }

    QString ALSAMIDIOutput::outputDeviceName() const
    {
        return d->m_currentOutput;
    }

    bool ALSAMIDIOutput::isMuted(int channel) const
    {
        if (channel >= 0 && channel < MIDI_CHANNELS)
            return d->m_muted[channel];
        return false;
    }

    MidiMapper* ALSAMIDIOutput::midiMap()
    {
        return d->m_mapper;
    }

    int ALSAMIDIOutput::pitchShift()
    {
        return d->m_pitchShift;
    }

    QStringList ALSAMIDIOutput::outputDeviceList(bool basicOnly)
    {
        d->m_clientFilter = basicOnly;
        reloadDeviceList();
        return d->m_outputDevices;
    }

    /* SLOTS */

    void ALSAMIDIOutput::setVolume(int channel, qreal value)
    {
        if (channel >= 0 && channel < MIDI_CHANNELS) {
            d->m_volumeShift[channel] = value;
            sendController(channel, MIDI_CTL_MSB_MAIN_VOLUME, d->m_volume[channel]);
            emit volumeChanged( channel, value );
        } else if ( channel == -1 ) {
            for (int chan = 0; chan < MIDI_CHANNELS; ++chan) {
                d->m_volumeShift[chan] = value;
                sendController(chan, MIDI_CTL_MSB_MAIN_VOLUME, d->m_volume[chan]);
                emit volumeChanged( chan, value );
            }
        }
    }

    void ALSAMIDIOutput::reloadDeviceList()
    {
        d->m_outputDevices.clear();
        QListIterator<PortInfo> it(d->m_client->getAvailableOutputs());
        while(it.hasNext()) {
            PortInfo p = it.next();
            QString name = QString("%1:%2").arg(p.getClientName()).arg(p.getPort());
            if (d->m_clientFilter && d->clientIsAdvanced(p.getClient()))
                continue;
            if ( d->m_clientFilter &&
                 name.startsWith(QLatin1String("Virtual Raw MIDI")) )
                continue;
            if ( name.startsWith(QLatin1String("KMid")) )
                continue;
            d->m_outputDevices << name;
        }
        if (!d->m_currentOutput.isEmpty() &&
            !d->m_outputDevices.contains(d->m_currentOutput)) {
            d->m_currentOutput.clear();
            emit outputDeviceChanged(d->m_currentOutput);
        }
    }

    bool ALSAMIDIOutput::setOutputDevice(int index)
    {
        if (index >= 0 && index < d->m_outputDevices.count())
            return setOutputDeviceName(d->m_outputDevices[index]);
        return false;
    }

    bool ALSAMIDIOutput::setOutputDeviceName(const QString &newOutputDevice)
    {
        if (d->m_outputDevices.contains(newOutputDevice)) {
            d->m_currentOutput = newOutputDevice;
            d->m_port->unsubscribeAll();
            d->m_port->subscribeTo(newOutputDevice);
            emit outputDeviceChanged(d->m_currentOutput);
            return true;
        }
        return false;
    }

    void ALSAMIDIOutput::setMuted(int channel, bool mute)
    {
        if (channel >= 0 && channel < MIDI_CHANNELS) {
            if (d->m_muted[channel] != mute) {
                if (mute) {
                    sendController(channel, MIDI_CTL_ALL_NOTES_OFF, 0);
                    sendController(channel, MIDI_CTL_ALL_SOUNDS_OFF, 0);
                }
                d->m_muted[channel] = mute;
                emit mutedChanged( channel, mute );
            }
        }
    }

    void ALSAMIDIOutput::setLocked(int channel, bool lock)
    {
        if (channel >= 0 && channel < MIDI_CHANNELS) {
            if (d->m_locked[channel] != lock) {
                d->m_locked[channel] = lock;
                if (lock)
                    d->m_lockedpgm[channel] = d->m_lastpgm[channel];
                emit lockedChanged( channel, lock );
            }
        }
    }

    void ALSAMIDIOutput::setMidiMap(MidiMapper *map)
    {
        d->m_mapper = map;
    }

    void ALSAMIDIOutput::setPitchShift(int amt)
    {
        if (d->m_pitchShift != amt) {
            allNotesOff();
            d->m_pitchShift = amt;
        }
    }

    void ALSAMIDIOutput::setResetMessage(const QByteArray& msg)
    {
        d->m_resetMessage = msg;
    }

    /* Realtime MIDI slots */

    void ALSAMIDIOutput::allNotesOff()
    {
        for(int chan = 0; chan < MIDI_CHANNELS; ++chan) {
            sendController(chan, MIDI_CTL_ALL_NOTES_OFF, 0);
            sendController(chan, MIDI_CTL_ALL_SOUNDS_OFF, 0);
        }
    }

    void ALSAMIDIOutput::resetControllers()
    {
        for(int chan = 0; chan < MIDI_CHANNELS; ++chan) {
            sendController(chan, MIDI_CTL_RESET_CONTROLLERS, 0);
            sendController(chan, MIDI_CTL_MSB_MAIN_VOLUME, 100);
        }
    }

    void ALSAMIDIOutput::sendResetMessage()
    {
        if (d->m_resetMessage.size() > 0)
            sendSysexEvent(d->m_resetMessage);
    }

    void ALSAMIDIOutput::sendEvent(SequencerEvent *ev, bool discardable)
    {
        QMutexLocker locker(&d->m_outMutex);
        d->transformEvent(ev);
        bool discard(false);
        if (SequencerEvent::isChannel(ev)) {
            ChannelEvent *cev = static_cast<ChannelEvent*>(ev);
            discard = discardable &&
                      ( d->m_muted[ cev->getChannel() ] ||
                        ( (cev->getSequencerType() == SND_SEQ_EVENT_PGMCHANGE)
                           && d->m_locked[ cev->getChannel() ] ) );
        }
        if (!discard) {
            ev->setSource(d->m_portId);
            ev->setSubscribers();
            ev->setDirect();
            d->m_client->outputDirect(ev);
        }
    }

    void ALSAMIDIOutput::sendNoteOn(int chan, int note, int vel)
    {
        NoteOnEvent ev(chan, note, vel);
        sendEvent(&ev);
    }

    void ALSAMIDIOutput::sendNoteOff(int chan, int note, int vel)
    {
        NoteOffEvent ev(chan, note, vel);
        sendEvent(&ev);
    }

    void ALSAMIDIOutput::sendController(int chan, int control, int value)
    {
        ControllerEvent ev(chan, control, value);
        sendEvent(&ev);
    }

    void ALSAMIDIOutput::sendKeyPressure(int chan, int note, int value)
    {
        KeyPressEvent ev(chan, note, value);
        sendEvent(&ev);
    }

    void ALSAMIDIOutput::sendProgram(int chan, int program)
    {
        ProgramChangeEvent ev(chan, program);
        sendEvent(&ev);
    }

    void ALSAMIDIOutput::sendChannelPressure(int chan, int value)
    {
        ChanPressEvent ev(chan, value);
        sendEvent(&ev);
    }

    void ALSAMIDIOutput::sendPitchBend(int chan, int value)
    {
        PitchBendEvent ev(chan, value);
        sendEvent(&ev);
    }

    void ALSAMIDIOutput::sendSysexEvent(const QByteArray& data)
    {
        SysExEvent ev(data);
        sendEvent(&ev);
    }

    MidiClient* ALSAMIDIOutput::client() const
    {
        return 0;
    }

    MidiPort* ALSAMIDIOutput::loopbackPort()
    {
        return 0;
    }

    void ALSAMIDIOutput::sendInitialProgram(int chan, int program)
    {
        int pgm(d->m_locked[chan] ? d->m_lockedpgm[chan] : program);
        if (pgm > -1) {
            ProgramChangeEvent ev(chan, pgm);
            sendEvent(&ev, false);
        }
    }

}
