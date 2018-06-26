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

#include "alsamidiobject.h"
#include "alsamidioutput.h"
#include "song.h"
#include "player.h"

#include <cmath>
#include <qsmf.h>
#include <alsaevent.h>
#include <alsaqueue.h>

#include <KIO/NetAccess>
#include <QTextStream>
#include <QTextCodec>
#include <QTime>
#include <QMutex>
#include <QMutexLocker>

using namespace drumstick;

namespace KMid {

    class ALSAMIDIObject::ALSAMIDIObjectPrivate {
    public:
        ALSAMIDIObjectPrivate() :
            m_out(0),
            m_client(0),
            m_port(0),
            m_queue(0),
            m_player(0),
            m_engine(0),
            m_codec(0),
            m_state(BufferingState),
            m_portId(-1),
            m_queueId(-1),
            m_clientId(-1),
            m_initialTempo(0),
            m_playlistIndex(-1),
            m_tempoFactor(1.0),
            m_lastTempo(0),
            m_tick(0),
            m_duration(0),
            m_lastBeat(0),
            m_beatLength(0),
            m_beatMax(0),
            m_barCount(0),
            m_beatCount(0),
            m_lowestMidiNote(127),
            m_highestMidiNote(0)
        {
            for(int i=0; i<MIDI_CHANNELS; ++i) {
                m_channelUsed[i] = false;
                m_channelEvents[i] = 0;
                m_channelPatches[i] = -1;
            }
        }

        virtual ~ALSAMIDIObjectPrivate()
        {
            if (m_client != NULL) {
                m_client->stopSequencerInput();
                if (m_port != NULL)
                    m_port->detach();
                m_client->close();
            }
            delete m_player;
        }

        void setQueueTempo() {
            m_queue->start();
            m_queue->stop();
            QueueTempo firstTempo = m_queue->getTempo();
            firstTempo.setPPQ(m_song.getDivision());
            firstTempo.setTempo(m_initialTempo);
            firstTempo.setTempoFactor(m_tempoFactor);
            m_queue->setTempo(firstTempo);
            m_client->drainOutput();
        }

        ALSAMIDIOutput *m_out;
        MidiClient *m_client;
        MidiPort *m_port;
        MidiQueue *m_queue;
        Player* m_player;
        QSmf *m_engine;
        QTextCodec *m_codec;

        State m_state;
        int m_portId;
        int m_queueId;
        int m_clientId;
        int m_initialTempo;
        int m_playlistIndex;
        qreal m_tempoFactor;
        qreal m_lastTempo;
        qint64 m_tick;
        Song m_song;
        QStringList m_loadingMessages;
        QStringList m_playList;
        QString m_encoding;
        qreal m_duration;
        qint64 m_lastBeat;
        qint64 m_beatLength;
        int m_beatMax;
        int m_barCount;
        int m_beatCount;
        int m_lowestMidiNote;
        int m_highestMidiNote;
        bool m_channelUsed[MIDI_CHANNELS];
        QMutex m_openMutex;
        int m_channelEvents[MIDI_CHANNELS];
        QByteArray m_trackLabel;
        QByteArray m_channelLabel[MIDI_CHANNELS];
        int m_channelPatches[MIDI_CHANNELS];
    };

    ALSAMIDIObject::ALSAMIDIObject(QObject *parent) : MIDIObject(parent),
        d(new ALSAMIDIObjectPrivate)
    {
        d->m_engine = new QSmf(this);
        connect(d->m_engine, SIGNAL(signalSMFHeader(int,int,int)), SLOT(headerEvent(int,int,int)));
        connect(d->m_engine, SIGNAL(signalSMFNoteOn(int,int,int)), SLOT(noteOnEvent(int,int,int)));
        connect(d->m_engine, SIGNAL(signalSMFNoteOff(int,int,int)), SLOT(noteOffEvent(int,int,int)));
        connect(d->m_engine, SIGNAL(signalSMFKeyPress(int,int,int)), SLOT(keyPressEvent(int,int,int)));
        connect(d->m_engine, SIGNAL(signalSMFCtlChange(int,int,int)), SLOT(ctlChangeEvent(int,int,int)));
        connect(d->m_engine, SIGNAL(signalSMFPitchBend(int,int)), SLOT(pitchBendEvent(int,int)));
        connect(d->m_engine, SIGNAL(signalSMFProgram(int,int)), SLOT(programEvent(int,int)));
        connect(d->m_engine, SIGNAL(signalSMFChanPress(int,int)), SLOT(chanPressEvent(int,int)));
        connect(d->m_engine, SIGNAL(signalSMFSysex(const QByteArray&)), SLOT(sysexEvent(const QByteArray&)));
        connect(d->m_engine, SIGNAL(signalSMFMetaMisc(int,const QByteArray&)), SLOT(metaEvent(int,const QByteArray&)));
        connect(d->m_engine, SIGNAL(signalSMFTempo(int)), SLOT(tempoEvent(int)));
        connect(d->m_engine, SIGNAL(signalSMFTrackStart()), SLOT(slotTrackStart()));
        connect(d->m_engine, SIGNAL(signalSMFTrackEnd()), SLOT(slotTrackEnd()));
        connect(d->m_engine, SIGNAL(signalSMFendOfTrack()), SLOT(endOfTrackEvent()));
        connect(d->m_engine, SIGNAL(signalSMFError(const QString&)), SLOT(errorHandler(const QString&)));
        connect(d->m_engine, SIGNAL(signalSMFTimeSig(int,int,int,int)), SLOT(timeSigEvent(int,int,int,int)));
    }

    ALSAMIDIObject::~ALSAMIDIObject()
    {
        delete d;
    }

    void ALSAMIDIObject::initialize(ALSAMIDIOutput *output)
    {
        d->m_out = output;
        d->m_client = new MidiClient(this);
        d->m_client->open();
        d->m_client->setClientName("KMid Internal");
        d->m_queue = d->m_client->getQueue();
        d->m_port = d->m_client->createPort();
        d->m_port->setPortName("loopback");
        d->m_port->setCapability( SND_SEQ_PORT_CAP_WRITE |
                                  SND_SEQ_PORT_CAP_NO_EXPORT );
        d->m_port->setPortType( SND_SEQ_PORT_TYPE_SPECIFIC );
        d->m_clientId = d->m_client->getClientId();
        d->m_queueId = d->m_queue->getId();
        d->m_portId = d->m_port->getPortId();
        d->m_port->setTimestamping(true);
        d->m_port->setTimestampReal(false);
        d->m_port->setTimestampQueue(d->m_queueId);
        d->m_port->subscribeFromAnnounce();
        d->m_player = new Player(d->m_client, d->m_portId);
        connect( d->m_player, SIGNAL(finished()),
                 SLOT(songFinished()), Qt::QueuedConnection );
        connect( d->m_player, SIGNAL(stopped()),
                 d->m_out, SLOT(allNotesOff()), Qt::QueuedConnection );
        d->m_client->setHandler(this);
        d->m_client->startSequencerInput();
    }

    void ALSAMIDIObject::handleSequencerEvent(SequencerEvent* ev)
    {
        if ( !SequencerEvent::isConnectionChange(ev) &&
             (d->m_state == PlayingState) )
            switch(ev->getSequencerType()) {
            case SND_SEQ_EVENT_ECHO: {
                    emit tick(ev->getTick());
                    qreal rtempo = currentTempo();
                    if (rtempo != d->m_lastTempo) {
                        emit tempoChanged(rtempo);
                        d->m_lastTempo = rtempo;
                    }
                }
                break;
            case SND_SEQ_EVENT_USR8:
                emit beat(ev->getRaw32(0), ev->getRaw8(4), ev->getRaw8(5));
                break;
            case SND_SEQ_EVENT_TIMESIGN:
                emit timeSignatureChanged(ev->getRaw8(0), ::pow(2, ev->getRaw8(1)));
                break;
            case SND_SEQ_EVENT_USR_VAR0: {
                    VariableEvent *ve = static_cast<VariableEvent*>(ev);
                    if (ve != NULL && ve->getData() != NULL && ve->getLength() > 0) {
                        QByteArray ba(ve->getData(), ve->getLength());
                        QString s;
                        if (d->m_codec == NULL)
                            s = QString::fromAscii(ba);
                        else
                            s = d->m_codec->toUnicode(ba);
                        s.remove(QRegExp("[/\\\\]+"));
                        s.remove(QRegExp("[\r\n]+"));
                        emit midiText(Song::Lyric, s);
                    }
                }
                break;
            case SND_SEQ_EVENT_NOTEOFF: {
                    d->m_out->sendEvent(ev);
                    const NoteOffEvent* n = static_cast<const NoteOffEvent*>(ev);
                    emit midiNoteOff(n->getChannel(), n->getKey(), n->getVelocity());
                }
                break;
            case SND_SEQ_EVENT_NOTEON: {
                    d->m_out->sendEvent(ev);
                    const NoteOnEvent* n = static_cast<const NoteOnEvent*>(ev);
                    emit midiNoteOn(n->getChannel(), n->getKey(), n->getVelocity());
                }
                break;
            case SND_SEQ_EVENT_KEYPRESS: {
                    d->m_out->sendEvent(ev);
                    const KeyPressEvent* n = static_cast<const KeyPressEvent*>(ev);
                    emit midiKeyPressure(n->getChannel(), n->getKey(), n->getVelocity());
                }
                break;
            case SND_SEQ_EVENT_CONTROLLER:
            case SND_SEQ_EVENT_CONTROL14: {
                    d->m_out->sendEvent(ev);
                    const ControllerEvent* n = static_cast<const ControllerEvent*>(ev);
                    emit midiController(n->getChannel(), n->getParam(), n->getValue());
                }
                break;
            case SND_SEQ_EVENT_PGMCHANGE: {
                    d->m_out->sendEvent(ev);
                    const ProgramChangeEvent* p = static_cast<const ProgramChangeEvent*>(ev);
                    emit midiProgram(p->getChannel(), p->getValue());
                }
                break;
            case SND_SEQ_EVENT_CHANPRESS: {
                    d->m_out->sendEvent(ev);
                    const ChanPressEvent* n = static_cast<const ChanPressEvent*>(ev);
                    emit midiChannelPressure(n->getChannel(), n->getValue());
                }
                break;
            case SND_SEQ_EVENT_PITCHBEND: {
                    d->m_out->sendEvent(ev);
                    const PitchBendEvent* n = static_cast<const PitchBendEvent*>(ev);
                    emit midiPitchBend(n->getChannel(), n->getValue());
                }
                break;
            default:
                d->m_out->sendEvent(ev);
            }
        delete ev;
    }

    State ALSAMIDIObject::state() const
    {
        return d->m_state;
    }

    qint32 ALSAMIDIObject::tickInterval() const
    {
        return d->m_player->getEchoResolution();
    }

    qint64 ALSAMIDIObject::currentTime() const
    {
        return d->m_queue->getStatus().getTickTime();
    }

    qreal ALSAMIDIObject::duration() const
    {
        return d->m_duration;
    }

    qint64 ALSAMIDIObject::remainingTime() const
    {
        if (d->m_song.isEmpty())
            return 0;
        return totalTime() - currentTime();
    }

    QString ALSAMIDIObject::errorString() const
    {
        return d->m_loadingMessages.join(QString(QChar::LineSeparator));
    }

    QStringList ALSAMIDIObject::metaData(const QString & key) const
    {
        if (key == "SMF_TEXT")
            return d->m_song.getText(Song::Text);
        else if (key == "SMF_COPYRIGHT")
            return d->m_song.getText(Song::Copyright);
        else if (key == "SMF_TRACKNAMES")
            return d->m_song.getText(Song::TrackName);
        else if (key == "SMF_INSTRUMENTNAMES")
            return d->m_song.getText(Song::InstrumentName);
        else if (key == "SMF_LYRICS")
            return d->m_song.getText(Song::Lyric);
        else if (key == "SMF_MARKERS")
            return d->m_song.getText(Song::Marker);
        else if (key == "SMF_CUES")
            return d->m_song.getText(Song::Cue);
        else if (key == "KAR_FILETYPE")
            return d->m_song.getText(Song::KarFileType);
        else if (key == "KAR_VERSION")
            return d->m_song.getText(Song::KarVersion);
        else if (key == "KAR_INFORMATION")
            return d->m_song.getText(Song::KarInformation);
        else if (key == "KAR_LANGUAGE")
            return d->m_song.getText(Song::KarLanguage);
        else if (key == "KAR_TITLES")
            return d->m_song.getText(Song::KarTitles);
        else if (key == "KAR_WARNINGS")
            return d->m_song.getText(Song::KarWarnings);
        return QStringList();
    }

    qint64 ALSAMIDIObject::totalTime() const
    {
        if (d->m_song.isEmpty()) return 0;
        return d->m_song.last()->getTick();
    }

    QString ALSAMIDIObject::currentSource() const
    {
        if (d->m_song.isEmpty()) return QString();
        return d->m_song.getFileName();
    }

    void ALSAMIDIObject::setCurrentSource(const QString &source)
    {
        if (d->m_playList.contains(source)) {
            d->m_playlistIndex = d->m_playList.indexOf(source);
        } else {
            d->m_playList.clear();
            d->m_playList << source;
            d->m_playlistIndex = 0;
        }
        openFile(source);
    }

    QStringList ALSAMIDIObject::queue() const
    {
        return d->m_playList;
    }

    void ALSAMIDIObject::setQueue(const QStringList &sources)
    {
        d->m_playList = sources;
    }

    void ALSAMIDIObject::setQueue(const QList<QUrl> &urls)
    {
        d->m_playList.clear();
        enqueue(urls);
    }

    void ALSAMIDIObject::enqueue(const QString &source)
    {
        d->m_playList.append( source );
    }

    void ALSAMIDIObject::enqueue(const QStringList &sources)
    {
        d->m_playList += sources;
    }

    void ALSAMIDIObject::enqueue(const QList<QUrl> &urls)
    {
        foreach(const QUrl &u, urls) {
            d->m_playList.append( u.toString() );
        }
    }

    void ALSAMIDIObject::clearQueue()
    {
        d->m_playList.clear();
        d->m_playlistIndex = -1;
    }

    qreal ALSAMIDIObject::timeSkew()
    {
        return d->m_tempoFactor;
    }

    /* SLOTS */

    void ALSAMIDIObject::setTickInterval(qint32 interval)
    {
        d->m_player->setEchoResolution(interval);
    }

    void ALSAMIDIObject::play()
    {
        if (!d->m_song.isEmpty() && !d->m_player->isRunning()) {
            if (currentTime() == 0) {
                if (d->m_initialTempo != 0)
                    d->setQueueTempo();
                d->m_out->sendResetMessage();
                d->m_out->resetControllers();
                sendInitialProgramChanges();
                d->m_lastBeat = 0;
                d->m_lastTempo = 0;
            }
            d->m_player->start();
            updateState( PlayingState );
        }
    }

    void ALSAMIDIObject::pause()
    {
        if (d->m_player->isRunning()) {
            d->m_player->stop();
            qint64 time = d->m_queue->getStatus().getTickTime();
            d->m_player->setPosition(time);
            updateState( PausedState );
        }
    }

    void ALSAMIDIObject::stop()
    {
        if (d->m_player->isRunning() || (d->m_state == PausedState)) {
            updateState( StoppedState );
            d->m_player->stop();
            d->m_queue->stop();
            d->m_queue->clear();
            d->m_player->resetPosition();
            d->m_queue->setTickPosition(0);
            d->m_client->drainOutput();
            d->m_lastBeat = 0;
            emit tick(0);
        }
    }

    void ALSAMIDIObject::seek(qint64 time)
    {
        if ( !(time < 0) && !d->m_song.empty() &&
             (time < d->m_song.last()->getTick()) ) {
            bool running = (d->m_state == PlayingState);
            if (running) {
                d->m_player->stop();
                updateState( PausedState );
            }
            d->m_player->setPosition(time);
            d->m_queue->setTickPosition(time);
            if (running) {
                d->m_player->start();
                updateState( PlayingState );
            }
        }
    }

    void ALSAMIDIObject::clear()
    {
        d->m_song.clear();
        clearQueue();
    }

    void ALSAMIDIObject::setTimeSkew(qreal skew)
    {
        if (d->m_tempoFactor != skew) {
            d->m_tempoFactor = skew;
            QueueTempo qtempo = d->m_queue->getTempo();
            qtempo.setTempoFactor(d->m_tempoFactor);
            d->m_queue->setTempo(qtempo);
            d->m_client->drainOutput();
        }
    }

    void ALSAMIDIObject::appendEvent(SequencerEvent* ev)
    {
        unsigned long tick = d->m_engine->getCurrentTime();
        ev->setSource(d->m_portId);
        ev->scheduleTick(d->m_queueId, tick, false);
        if (ev->getSequencerType() != SND_SEQ_EVENT_TEMPO)
            ev->setDestination(d->m_clientId, d->m_portId);
        d->m_song.append(ev);
        updateLoadProgress();
    }

    void ALSAMIDIObject::headerEvent(int format, int ntrks, int division)
    {
        d->m_song.setHeader(format, ntrks, division);
        d->m_beatLength = division;
        d->m_beatMax = 4;
        d->m_lastBeat = 0;
        d->m_beatCount = 1;
        d->m_barCount = 1;
        updateLoadProgress();
    }

    void ALSAMIDIObject::noteOnEvent(int chan, int pitch, int vol)
    {
        if (pitch > d->m_highestMidiNote)
            d->m_highestMidiNote = pitch;
        if (pitch < d->m_lowestMidiNote)
            d->m_lowestMidiNote = pitch;
        d->m_channelUsed[chan] = true;
        d->m_channelEvents[chan]++;
        SequencerEvent* ev = new NoteOnEvent (chan, pitch, vol);
        appendEvent(ev);
    }

    void ALSAMIDIObject::noteOffEvent(int chan, int pitch, int vol)
    {
        if (pitch > d->m_highestMidiNote)
            d->m_highestMidiNote = pitch;
        if (pitch < d->m_lowestMidiNote)
            d->m_lowestMidiNote = pitch;
        d->m_channelUsed[chan] = true;
        d->m_channelEvents[chan]++;
        SequencerEvent* ev = new NoteOffEvent (chan, pitch, vol);
        appendEvent(ev);
    }

    void ALSAMIDIObject::keyPressEvent(int chan, int pitch, int press)
    {
        d->m_channelUsed[chan] = true;
        d->m_channelEvents[chan]++;
        SequencerEvent* ev = new KeyPressEvent (chan, pitch, press);
        appendEvent(ev);
    }

    void ALSAMIDIObject::ctlChangeEvent(int chan, int ctl, int value)
    {
        d->m_channelUsed[chan] = true;
        d->m_channelEvents[chan]++;
        SequencerEvent* ev = new ControllerEvent (chan, ctl, value);
        appendEvent(ev);
    }

    void ALSAMIDIObject::pitchBendEvent(int chan, int value)
    {
        d->m_channelUsed[chan] = true;
        d->m_channelEvents[chan]++;
        SequencerEvent* ev = new PitchBendEvent (chan, value);
        appendEvent(ev);
    }

    void ALSAMIDIObject::programEvent(int chan, int patch)
    {
        d->m_channelUsed[chan] = true;
        d->m_channelEvents[chan]++;
        if (d->m_channelPatches[chan] < 0)
            d->m_channelPatches[chan] = patch;
        SequencerEvent* ev = new ProgramChangeEvent (chan, patch);
        appendEvent(ev);
    }

    void ALSAMIDIObject::chanPressEvent(int chan, int press)
    {
        d->m_channelUsed[chan] = true;
        d->m_channelEvents[chan]++;
        SequencerEvent* ev = new ChanPressEvent (chan, press);
        appendEvent(ev);
    }

    void ALSAMIDIObject::sysexEvent(const QByteArray& data)
    {
        SequencerEvent* ev = new SysExEvent (data);
        appendEvent(ev);
    }

    void ALSAMIDIObject::metaEvent(int type, const QByteArray& data)
    {
        if ( (type >= Song::FIRST_TYPE) && (type <= Song::Cue) ) {
            qint64 tick = d->m_engine->getCurrentTime();
            d->m_song.addMetaData(static_cast<Song::TextType>(type), data, tick);
            switch ( type ) {
            case Song::Lyric:
            case Song::Text:
                if ((data.length() > 0) && (data[0] != '@') && (data[0] != '%') ) {
                    VariableEvent *ev = new VariableEvent(data);
                    ev->setSequencerType(SND_SEQ_EVENT_USR_VAR0);
                    appendEvent(ev);
                }
                break;
            case Song::TrackName:
            case Song::InstrumentName:
                if (d->m_trackLabel.isEmpty())
                    d->m_trackLabel = data;
                break;
            }
        }
    }

    void ALSAMIDIObject::tempoEvent(int tempo)
    {
        if ( d->m_initialTempo == 0 )
            d->m_initialTempo = tempo;
        SequencerEvent* ev = new TempoEvent (d->m_queueId, tempo);
        appendEvent(ev);
    }

    void ALSAMIDIObject::endOfTrackEvent()
    {
        qreal seconds = d->m_engine->getRealTime() / 1600.0;
        if (seconds > d->m_duration) d->m_duration = seconds;
    }

    void ALSAMIDIObject::timeSigEvent(int b0, int b1, int b2, int b3)
    {
        SequencerEvent* ev = new SequencerEvent();
        ev->setSequencerType(SND_SEQ_EVENT_TIMESIGN);
        ev->setRaw8(0, b0);
        ev->setRaw8(1, b1);
        ev->setRaw8(2, b2);
        ev->setRaw8(3, b3);
        appendEvent(ev);
        d->m_beatMax = b0;
        d->m_beatLength = d->m_song.getDivision() * 4 / ::pow(2, b1);
    }

    void ALSAMIDIObject::errorHandler(const QString& errorStr)
    {
        d->m_loadingMessages << QString("%1 at file offset %2<br>")
            .arg(errorStr).arg(d->m_engine->getFilePos());
    }

    void ALSAMIDIObject::updateLoadProgress()
    {
        qint64 ticks = d->m_engine->getCurrentTime();
        if (ticks > d->m_tick) {
            qint64 diff = ticks - d->m_lastBeat;
            while (diff >= d->m_beatLength) {
                SequencerEvent* ev = new SequencerEvent();
                ev->setSequencerType(SND_SEQ_EVENT_USR8);
                ev->setRaw32(0, d->m_barCount);
                ev->setRaw8(4, d->m_beatCount);
                ev->setRaw8(5, d->m_beatMax);
                ev->setSource(d->m_portId);
                ev->scheduleTick(d->m_queueId, d->m_lastBeat, false);
                ev->setDestination(d->m_clientId, d->m_portId);
                d->m_song.append(ev);

                d->m_lastBeat += d->m_beatLength;
                diff -= d->m_beatLength;
                d->m_beatCount++;
                if (d->m_beatCount > d->m_beatMax) {
                    d->m_beatCount = 1;
                    d->m_barCount++;
                }
            }
            d->m_tick = ticks;
        }
    }

    void ALSAMIDIObject::openFile(const QString &fileName)
    {
        QMutexLocker locker(&d->m_openMutex);
        QString tmpFile;
        if(KIO::NetAccess::download(fileName, tmpFile, 0)) {
            updateState( LoadingState );
            d->m_song.clear();
            d->m_loadingMessages.clear();
            d->m_tick = 0;
            d->m_initialTempo = 0;
            d->m_duration = 0;
            d->m_lastBeat = 0;
            d->m_barCount = 0;
            d->m_beatCount = 0;
            d->m_beatMax = 4;
            d->m_lowestMidiNote = 127;
            d->m_highestMidiNote = 0;
            for(int i=0; i<MIDI_CHANNELS; ++i) {
                d->m_channelUsed[i] = false;
                d->m_channelEvents[i] = 0;
                d->m_channelLabel[i].clear();
                d->m_channelPatches[i] = -1;
            }
            try {
                d->m_engine->readFromFile(tmpFile);
                if (!d->m_song.isEmpty()) {
                    d->m_song.sort();
                    addSongPadding();
                    if (d->m_initialTempo == 0)
                        d->m_initialTempo = 500000;
                    d->m_song.setFileName(fileName);
                    d->m_player->setSong(&d->m_song);
                    d->setQueueTempo();
                    d->m_player->resetPosition();
                    setTickInterval(d->m_song.getDivision() / 6);
                    updateState( StoppedState );
                    emit currentSourceChanged(fileName);
                }
            } catch (...) {
                d->m_song.clear();
                updateState( ErrorState );
            }
            KIO::NetAccess::removeTempFile(tmpFile);
        } else {
            d->m_loadingMessages << KIO::NetAccess::lastErrorString();
            updateState( ErrorState );
        }
    }

    void ALSAMIDIObject::songFinished()
    {
        updateState( StoppedState );
        d->m_player->resetPosition();
        d->m_lastBeat = 0;
        d->m_out->allNotesOff();
        bool goNext = d->m_playlistIndex < d->m_playList.count()-1;
        emit finished();
        if (goNext && (d->m_playlistIndex < d->m_playList.count()-1))
            setCurrentSource(d->m_playList.at(d->m_playlistIndex+1));
    }

    void ALSAMIDIObject::updateState(State newState)
    {
        State oldState = d->m_state;
        if (oldState != newState) {
            d->m_state = newState;
            emit stateChanged(newState, oldState);
        }
    }

    QString ALSAMIDIObject::getTextEncoding() const
    {
        return d->m_encoding;
    }

    void ALSAMIDIObject::setTextEncoding(const QString& encoding)
    {
        if (encoding != d->m_encoding) {
            if(encoding.isEmpty())
                d->m_codec = NULL;
            else
                d->m_codec = QTextCodec::codecForName(encoding.toLatin1());
            d->m_song.setTextCodec(d->m_codec);
            d->m_encoding = encoding;
        }
    }

    QStringList ALSAMIDIObject::getLyrics(qint64 time) const
    {
        return d->m_song.getLyrics(time);
    }

    qreal ALSAMIDIObject::currentTempo()
    {
        return d->m_queue->getTempo().getRealBPM();
    }

    bool ALSAMIDIObject::channelUsed(int channel)
    {
        if (channel >= 0 && channel < MIDI_CHANNELS)
            return d->m_channelUsed[channel];
        return false;
    }

    int ALSAMIDIObject::lowestMidiNote()
    {
        return d->m_lowestMidiNote;
    }

    int ALSAMIDIObject::highestMidiNote()
    {
        return d->m_highestMidiNote;
    }

    void ALSAMIDIObject::addSongPadding()
    {
        unsigned long tick = d->m_song.last()->getTick();
        tick += (d->m_beatMax * d->m_beatLength); // a full bar
        SequencerEvent* ev = new SystemEvent(SND_SEQ_EVENT_ECHO);
        ev->setSource(d->m_portId);
        ev->scheduleTick(d->m_queueId, tick, false);
        ev->setDestination(d->m_clientId, d->m_portId);
        d->m_song.append(ev);
        updateLoadProgress();
    }

    bool ALSAMIDIObject::guessTextEncoding()
    {
        bool res = d->m_song.guessTextCodec();
        if (res && d->m_song.getTextCodec() != NULL)
            setTextEncoding(QString(d->m_song.getTextCodec()->name()));
        return res;
    }

    void ALSAMIDIObject::slotTrackStart()
    {
        for(int i=0; i<MIDI_CHANNELS; ++i)
            d->m_channelEvents[i] = 0;
        d->m_trackLabel.clear();
        updateLoadProgress();
    }

    void ALSAMIDIObject::slotTrackEnd()
    {
        int max = 0;
        int chan = -1;
        if (!d->m_trackLabel.isEmpty()) {
            for(int i=0; i<MIDI_CHANNELS; ++i)
                if (d->m_channelEvents[i] > max) {
                    max = d->m_channelEvents[i];
                    chan = i;
                }
            if (chan >= 0 && chan < MIDI_CHANNELS)
                d->m_channelLabel[chan] = d->m_trackLabel;
        }
        updateLoadProgress();
    }

    QString ALSAMIDIObject::channelLabel(int channel)
    {
        if (channel >= 0 && channel < MIDI_CHANNELS) {
            if (d->m_codec == NULL)
                return QString::fromAscii(d->m_channelLabel[channel]);
            else
                return d->m_codec->toUnicode(d->m_channelLabel[channel]);
        }
        return QString();
    }

    QVariant ALSAMIDIObject::songProperty(const QString& key)
    {
        if (key == QLatin1String("SMF_FORMAT"))
            return QVariant(d->m_song.getFormat());
        else if (key == QLatin1String("SMF_TRACKS"))
            return QVariant(d->m_song.getTracks());
        else if (key == QLatin1String("SMF_DIVISION"))
            return QVariant(d->m_song.getDivision());
        else if (key == QLatin1String("NUM_BARS"))
            return QVariant(d->m_barCount);
        else if (key == QLatin1String("NUM_BEATS")) {
            int beats = d->m_song.last()->getTick() / d->m_song.getDivision();
            return QVariant(beats);
        }
        return QVariant();
    }

    QVariant ALSAMIDIObject::channelProperty(int channel, const QString& key)
    {
        if (channel >= 0 && channel < MIDI_CHANNELS) {
            if (key == QLatin1String("INITIAL_PATCH"))
                return QVariant(d->m_channelPatches[channel]);
            else if (key == QLatin1String("LABEL"))
                return QVariant(d->m_channelLabel[channel]);
            else if (key == QLatin1String("USED"))
                return QVariant(d->m_channelUsed[channel]);
        }
        return QVariant();
    }

    void ALSAMIDIObject::sendInitialProgramChanges()
    {
        for (int i = 0; i < MIDI_CHANNELS; ++i) {
            int patch(d->m_channelPatches[i]);
            d->m_out->sendInitialProgram(i, patch);
        }
    }

}
