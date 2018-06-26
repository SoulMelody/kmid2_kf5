/*
    KMid Dummy Backend
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

#include "dummymidioutput.h"
#include <QStringList>

namespace KMid {

    class DummyMIDIOutput::DummyMIDIOutputPrivate {
    public:
        DummyMIDIOutputPrivate() {}
        virtual ~DummyMIDIOutputPrivate() {}
    };

    DummyMIDIOutput::DummyMIDIOutput(QObject *parent) :
        MIDIOutput(parent), d(new DummyMIDIOutputPrivate)
    { }

    DummyMIDIOutput::~DummyMIDIOutput()
    {
        delete d;
    }

    qreal DummyMIDIOutput::volume(int /*channel*/) const
    {
        return -1.0;
    }

    int DummyMIDIOutput::outputDevice() const
    {
        return 0;
    }

    QString DummyMIDIOutput::outputDeviceName() const
    {
        return QString();
    }

    bool DummyMIDIOutput::isMuted(int /*channel*/) const
    {
        return false;
    }

    MidiMapper* DummyMIDIOutput::midiMap()
    {
        return NULL;
    }

    int DummyMIDIOutput::pitchShift()
    {
        return 0;
    }

    QStringList DummyMIDIOutput::outputDeviceList(bool /*basicOnly*/)
    {
        return QStringList();
    }

    /* SLOTS */

    void DummyMIDIOutput::setVolume(int /*channel*/, qreal /*value*/)
    {
    }

    void DummyMIDIOutput::reloadDeviceList()
    {
    }

    bool DummyMIDIOutput::setOutputDevice(int /*index*/)
    {
        return false;
    }

    bool DummyMIDIOutput::setOutputDeviceName(const QString& /*name*/ )
    {
        return false;
    }

    void DummyMIDIOutput::setMuted(int /*channel*/, bool /*mute*/)
    {
    }

    void DummyMIDIOutput::setLocked(int /*channel*/, bool /*lock*/)
    {
    }

    void DummyMIDIOutput::setMidiMap(MidiMapper* /*map*/)
    {
    }

    void DummyMIDIOutput::setPitchShift(int /*amt*/)
    {
    }

    void DummyMIDIOutput::setResetMessage(const QByteArray& /*msg*/)
    {
    }

    /* Realtime MIDI slots */

    void DummyMIDIOutput::allNotesOff()
    {
//        for(int chan = 0; chan < MIDI_CHANNELS; ++chan) {
//            sendController(chan, MIDI_CTL_ALL_NOTES_OFF, 0);
//            sendController(chan, MIDI_CTL_ALL_SOUNDS_OFF, 0);
//        }
    }

    void DummyMIDIOutput::resetControllers()
    {
//        for(int chan = 0; chan < MIDI_CHANNELS; ++chan) {
//            sendController(chan, MIDI_CTL_RESET_CONTROLLERS, 0);
//            sendController(chan, MIDI_CTL_MSB_MAIN_VOLUME, 100);
//        }
    }

    void DummyMIDIOutput::sendResetMessage()
    {
    }

    void DummyMIDIOutput::sendNoteOn(int /*chan*/, int /*note*/, int /*vel*/)
    {
    }

    void DummyMIDIOutput::sendNoteOff(int /*chan*/, int /*note*/, int /*vel*/)
    {
    }

    void DummyMIDIOutput::sendController(int /*chan*/, int /*control*/, int /*value*/)
    {
    }

    void DummyMIDIOutput::sendKeyPressure(int /*chan*/, int /*note*/, int /*value*/)
    {
    }

    void DummyMIDIOutput::sendProgram(int /*chan*/, int /*program*/)
    {
    }

    void DummyMIDIOutput::sendChannelPressure(int /*chan*/, int /*value*/)
    {
    }

    void DummyMIDIOutput::sendPitchBend(int /*chan*/, int /*value*/)
    {
    }

    void DummyMIDIOutput::sendSysexEvent(const QByteArray& /*data*/)
    {
    }

}

#include "dummymidioutput.moc"
