/*
    KMid Windows Backend
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

#include "winmidiobject.h"
#include "winmidioutput.h"
#include "midimapper.h"
#include "song.h"

#include <qsmf.h>
#include <KDebug>
#include <KIO/NetAccess>
#include <QTextStream>
#include <QTextCodec>
#include <QStringList>
#include <QMutex>
#include <QFileInfo>
#include <qmath.h>

using namespace drumstick;

namespace KMid {

    class WinMIDIObject::WinMIDIObjectPrivate {
    public:
        WinMIDIObjectPrivate():
            m_out(0),
            m_engine(0),
            m_codec(0),
            m_state(BufferingState),
            m_initialTempo(0),
            m_playlistIndex(-1),
            m_tick(0),
            m_duration(0),
            m_lastBeat(0),
            m_beatLength(0),
            m_beatMax(0),
            m_barCount(0),
            m_beatCount(0),
            m_lowestMidiNote(127),
            m_highestMidiNote(0),
            m_currentTicks(0)
        {
            m_dbgState[LoadingState]   = QLatin1String("Loading");
            m_dbgState[StoppedState]   = QLatin1String("Stopped");
            m_dbgState[PlayingState]   = QLatin1String("Playing");
            m_dbgState[BufferingState] = QLatin1String("Buffering");
            m_dbgState[PausedState]    = QLatin1String("Paused");
            m_dbgState[ErrorState]     = QLatin1String("Error");
        }

        virtual ~WinMIDIObjectPrivate()
        { }

        WinMIDIOutput *m_out;
        QSmf *m_engine;
        QTextCodec *m_codec;
        State m_state;
        int m_initialTempo;
        int m_playlistIndex;
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
        qint64 m_currentTicks;
        bool m_channelUsed[MIDI_CHANNELS];
        int m_channelEvents[MIDI_CHANNELS];
        QByteArray m_channelLabel[MIDI_CHANNELS];
        int m_channelPatches[MIDI_CHANNELS];
        QByteArray m_trackLabel;
        QMutex m_openMutex;
        QMap<uint,QByteArray> m_textUserEvents;
        QMap<State,QString> m_dbgState;

    };

    WinMIDIObject::WinMIDIObject(QObject *parent) :
            MIDIObject(parent), d(new WinMIDIObjectPrivate)
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

    void WinMIDIObject::initialize(WinMIDIOutput *output)
    {
        d->m_out = output;
        connect(d->m_out, SIGNAL(finished()), SLOT(songFinished()), Qt::QueuedConnection);
     }

    WinMIDIObject::~WinMIDIObject()
    {
        delete d;
    }

    State WinMIDIObject::state() const
    {
        return d->m_state;
    }

    qint32 WinMIDIObject::tickInterval() const
    {
        return d->m_out->tickInterval();
    }

    qint64 WinMIDIObject::currentTime() const
    {
        return d->m_out->currentTime();
    }

    qreal WinMIDIObject::duration() const
    {
        return d->m_duration;
    }

    qint64 WinMIDIObject::remainingTime() const
    {
        return totalTime() - currentTime();
    }

    QString WinMIDIObject::errorString() const
    {
        return d->m_loadingMessages.join(QString(QChar::LineSeparator));
    }

    QStringList WinMIDIObject::metaData(const QString& key) const
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

    qint64 WinMIDIObject::totalTime() const
    {
        if (d->m_song.isEmpty())
            return 0;
        return d->m_song.last()->getTick();
    }

    QString WinMIDIObject::currentSource() const
    {
        if ( !d->m_song.isEmpty() &&
             d->m_playlistIndex >=0 &&
             d->m_playlistIndex < d->m_playList.size() )
            return d->m_playList.at(d->m_playlistIndex);
        return QString();
    }

    void WinMIDIObject::setCurrentSource(const QString& source )
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

    QStringList WinMIDIObject::queue() const
    {
        return d->m_playList;
    }

    void WinMIDIObject::setQueue(const QStringList& sources)
    {
         d->m_playList = sources;
    }

    void WinMIDIObject::setQueue(const QList<QUrl>& urls )
    {
        d->m_playList.clear();
        enqueue(urls);
    }

    void WinMIDIObject::enqueue(const QString& source )
    {
        d->m_playList.append( source );
    }

    void WinMIDIObject::enqueue(const QStringList& sources )
    {
        d->m_playList += sources;
    }

    void WinMIDIObject::enqueue(const QList<QUrl>& urls )
    {
        foreach(const QUrl &u, urls) {
            d->m_playList.append( u.toString() );
        }
    }

    void WinMIDIObject::clearQueue()
    {
        d->m_playList.clear();
        d->m_playlistIndex = -1;
    }

    void WinMIDIObject::clearSong()
    {
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
        d->m_currentTicks = 0;
        d->m_textUserEvents.clear();
        for (int i=0; i<MIDI_CHANNELS; ++i) {
            d->m_channelUsed[i] = false;
            d->m_channelEvents[i] = 0;
            d->m_channelLabel[i].clear();
            d->m_channelPatches[i] = -1;
        }
    }

    qreal WinMIDIObject::timeSkew()
    {
         return d->m_out->timeSkew();
    }

    /* SLOTS */

    void WinMIDIObject::setTickInterval(qint32 interval )
    {
        d->m_out->setTickInterval(interval);
    }

    void WinMIDIObject::play()
    {
        if ( !d->m_song.isEmpty() &&
             (d->m_state == PausedState || d->m_state == StoppedState) ) {
            d->m_out->setTimeDivision(d->m_song.getDivision());
            if (currentTime() == 0)
                d->m_out->setInitialTempo(d->m_initialTempo);
            d->m_out->sendResetMessage();
            d->m_out->resetControllers();
            sendInitialProgramChanges();
            d->m_out->songOutput();
            if (d->m_out->play())
                updateState( PlayingState );
        }
    }

    void WinMIDIObject::pause()
    {
        if ( !d->m_song.isEmpty() && (d->m_state == PlayingState) ) {
            if (d->m_out->pause())
                updateState( PausedState );
        }
    }

    void WinMIDIObject::stop()
    {
        if ( !d->m_song.isEmpty() &&
             (d->m_state == PausedState || d->m_state == PlayingState) ) {
            if (d->m_out->stop()) {
                updateState( StoppedState );
                seek(0);
                emit tick(0);
            }
        }
    }

    void WinMIDIObject::seek(qint64 time)
    {
        if (!(time < 0) && !d->m_song.empty() &&
            (time < d->m_song.last()->getTick())) {
            bool running = (d->m_state == PlayingState);
            if (running) {
                d->m_out->pause();
                updateState(PausedState);
            }
            d->m_out->seek(time);
            if (running) {
                d->m_out->play();
                updateState( PlayingState );
            }
        }
    }

    void WinMIDIObject::clear()
    {
        clearSong();
        clearQueue();
    }

    void WinMIDIObject::setTimeSkew(qreal skew)
    {
        d->m_out->setTimeSkew(skew);
    }

    QString WinMIDIObject::getTextEncoding() const
    {
        return d->m_encoding;
    }

    void WinMIDIObject::setTextEncoding(const QString& encoding )
    {
        if (encoding != d->m_encoding) {
            if (encoding.isEmpty())
                d->m_codec = NULL;
            else
                d->m_codec = QTextCodec::codecForName(encoding.toLatin1());
            d->m_song.setTextCodec(d->m_codec);
            d->m_encoding = encoding;
        }
    }

    QStringList WinMIDIObject::getLyrics( qint64 time ) const
    {
         return d->m_song.getLyrics(time);
    }

    qreal WinMIDIObject::currentTempo()
    {
        return d->m_out->currentTempo();
    }

    bool WinMIDIObject::channelUsed(int channel )
    {
        return d->m_channelUsed[channel];
    }

    int WinMIDIObject::lowestMidiNote()
    {
        return d->m_lowestMidiNote;
    }

    int WinMIDIObject::highestMidiNote()
    {
         return d->m_highestMidiNote;
    }

    bool WinMIDIObject::guessTextEncoding()
    {
        bool res = d->m_song.guessTextCodec();
        if (res && d->m_song.getTextCodec() != NULL)
            setTextEncoding(QString(d->m_song.getTextCodec()->name()));
        return res;
    }

    QString WinMIDIObject::channelLabel(int channel )
    {
        if (channel >= 0 && channel < MIDI_CHANNELS) {
            if (d->m_codec == NULL)
                return QString::fromAscii(d->m_channelLabel[channel]);
            else
                return d->m_codec->toUnicode(d->m_channelLabel[channel]);
        }
        return QString();
    }

    /* SMF Parsing slots */

    void WinMIDIObject::appendEvent(SongEvent* ev)
    {
        d->m_currentTicks = d->m_engine->getCurrentTime();
        ev->setTick(d->m_currentTicks);
        d->m_song.append(ev);
        updateLoadProgress();
    }

    void WinMIDIObject::headerEvent(int format, int ntrks, int division)
    {
        d->m_song.setHeader(format, ntrks, division);
        d->m_beatLength = division;
        d->m_beatMax = 4;
        d->m_lastBeat = 0;
        d->m_beatCount = 1;
        d->m_barCount = 1;
        d->m_currentTicks = d->m_engine->getCurrentTime();
        setTickInterval(division / 6);
        updateLoadProgress();
    }

    void WinMIDIObject::noteOnEvent(int chan, int pitch, int vol)
    {
        if (pitch > d->m_highestMidiNote)
            d->m_highestMidiNote = pitch;
        if (pitch < d->m_lowestMidiNote)
            d->m_lowestMidiNote = pitch;
        d->m_channelUsed[chan] = true;
        d->m_channelEvents[chan]++;
        SongEvent* ev = new SongEvent ( SHORT_EVENT,
            MIDI_STATUS_NOTEON | (chan & MIDI_CHANNEL_MASK), pitch, vol );
        appendEvent(ev);
    }

    void WinMIDIObject::noteOffEvent(int chan, int pitch, int vol)
    {
        if (pitch > d->m_highestMidiNote)
            d->m_highestMidiNote = pitch;
        if (pitch < d->m_lowestMidiNote)
            d->m_lowestMidiNote = pitch;
        d->m_channelUsed[chan] = true;
        d->m_channelEvents[chan]++;
        SongEvent* ev = new SongEvent ( SHORT_EVENT,
            MIDI_STATUS_NOTEOFF | (chan & MIDI_CHANNEL_MASK), pitch, vol);
        appendEvent(ev);
    }

    void WinMIDIObject::keyPressEvent(int chan, int pitch, int press)
    {
        d->m_channelUsed[chan] = true;
        d->m_channelEvents[chan]++;
        SongEvent* ev = new SongEvent ( SHORT_EVENT,
            MIDI_STATUS_KEYPRESURE | (chan & MIDI_CHANNEL_MASK), pitch, press);
        appendEvent(ev);
    }

    void WinMIDIObject::ctlChangeEvent(int chan, int ctl, int value)
    {
        d->m_channelUsed[chan] = true;
        d->m_channelEvents[chan]++;
        SongEvent* ev = new SongEvent ( SHORT_EVENT,
            MIDI_STATUS_CONTROLCHANGE | (chan & MIDI_CHANNEL_MASK), ctl, value);
        appendEvent(ev);
    }

    void WinMIDIObject::pitchBendEvent(int chan, int value)
    {
        d->m_channelUsed[chan] = true;
        d->m_channelEvents[chan]++;
        SongEvent* ev = new SongEvent ( SHORT_EVENT,
            MIDI_STATUS_PITCHBEND | (chan & MIDI_CHANNEL_MASK), value);
        appendEvent(ev);
    }

    void WinMIDIObject::programEvent(int chan, int patch)
    {
        d->m_channelUsed[chan] = true;
        d->m_channelEvents[chan]++;
        if (d->m_channelPatches[chan] < 0)
            d->m_channelPatches[chan] = patch;
        SongEvent* ev = new SongEvent ( SHORT_EVENT,
            MIDI_STATUS_PROGRAMCHANGE | (chan & MIDI_CHANNEL_MASK), patch);
        appendEvent(ev);
    }

    void WinMIDIObject::chanPressEvent(int chan, int press)
    {
        d->m_channelUsed[chan] = true;
        d->m_channelEvents[chan]++;
        SongEvent* ev = new SongEvent ( SHORT_EVENT,
            MIDI_STATUS_CHANNELPRESSURE | (chan & MIDI_CHANNEL_MASK), press );
        appendEvent(ev);
    }

    void WinMIDIObject::sysexEvent(const QByteArray& data)
    {
        SongEvent* ev = new SongEvent ( SYSEX_EVENT, data );
        appendEvent(ev);
    }

    void WinMIDIObject::metaEvent(int type, const QByteArray& data)
    {
        if ( (type >= Song::FIRST_TYPE) && (type <= Song::Cue) ) {
            qint64 tick = d->m_engine->getCurrentTime();
            d->m_song.addMetaData(static_cast<Song::TextType>(type), data, tick);
            switch ( type ) {
            case Song::Lyric:
            case Song::Text:
                if ((data.length() > 0) && (data[0] != '@') && (data[0] != '%') ) {
                    if (d->m_textUserEvents.contains(tick))
                        d->m_textUserEvents[tick] += data;
                    else {
                        SongEvent *ev = new SongEvent( LYRIC_EVENT );
                        appendEvent(ev);
                        d->m_textUserEvents[tick] = data;
                    }
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

    void WinMIDIObject::tempoEvent(int tempo)
    {
        if ( d->m_initialTempo == 0 )
            d->m_initialTempo = tempo;
        SongEvent* ev = new SongEvent ( TEMPO_EVENT, tempo );
        appendEvent(ev);
    }

    void WinMIDIObject::endOfTrackEvent()
    {
        qreal seconds = d->m_engine->getRealTime() / 1600.0;
        if (seconds > d->m_duration) d->m_duration = seconds;
    }

    void WinMIDIObject::timeSigEvent(int b0, int b1, int /*b2*/, int /*b3*/)
    {
        int den = qPow(2, b1);
        d->m_currentTicks = d->m_engine->getCurrentTime();
        updateLoadProgress();
        d->m_beatMax = b0;
        d->m_beatLength = d->m_song.getDivision() * 4 / den;
    }

    void WinMIDIObject::errorHandler(const QString& errorStr)
    {
        d->m_loadingMessages << QString("%1 at file offset %2<br>")
            .arg(errorStr).arg(d->m_engine->getFilePos());
    }

    void WinMIDIObject::updateLoadProgress()
    {
        if (d->m_currentTicks > d->m_tick) {
            qint64 diff = d->m_currentTicks - d->m_lastBeat;
            while (diff >= d->m_beatLength) {
                SongEvent* ev = new SongEvent( BEAT_EVENT,
                                               d->m_barCount,
                                               d->m_beatCount,
                                               d->m_beatMax );
                ev->setTick(d->m_lastBeat);
                d->m_song.append(ev);
                qint64 tickTime = d->m_lastBeat + tickInterval();
                d->m_lastBeat += d->m_beatLength;
                while (tickTime < d->m_lastBeat) {
                    SongEvent* ev = new SongEvent( TICK_EVENT );
                    ev->setTick(tickTime);
                    d->m_song.append(ev);
                    tickTime += tickInterval();
                }
                diff -= d->m_beatLength;
                d->m_beatCount++;
                if (d->m_beatCount > d->m_beatMax) {
                    d->m_beatCount = 1;
                    d->m_barCount++;
                }
            }
            d->m_tick = d->m_currentTicks;
        }
    }

    void WinMIDIObject::openFile(const QString &fileName)
    {
        QFileInfo finfo;
        QString tmpFile = fileName;
        //TODO: looks like KIO::NetAccess::download() is broken in Windows/KDE 4.4.0
        KUrl url(fileName);
        kDebug() << fileName;
        if (fileName.startsWith(QLatin1String("file://"), Qt::CaseInsensitive)) {
            kDebug() << "KUrl::localFile:" << url.toLocalFile();
            tmpFile = fileName.mid(7);
            kDebug() << "workaround, loading" << tmpFile;
        } else {
            kDebug() << "download:" << url;
            KIO::NetAccess::download(url, tmpFile, 0);
        }
        d->m_openMutex.lock();
        finfo.setFile(tmpFile);
        if (finfo.exists() && finfo.size() > 0) {
            updateState( LoadingState );
            clearSong();
            try {
                d->m_engine->readFromFile(tmpFile);
                if (!d->m_song.isEmpty()) {
                    addSongPadding();
                    d->m_song.sort();
                    if (d->m_initialTempo == 0) {
                        d->m_initialTempo = 500000;
                    }
                    d->m_song.setFileName(fileName);
                    d->m_out->setSong(&d->m_song);
                    d->m_out->setInitialTempo(d->m_initialTempo);
                    d->m_out->resetPosition();
                    updateState( StoppedState );
                    emit currentSourceChanged(fileName);
                }
            } catch (...) {
                clearSong();
                updateState( ErrorState );
            }
            if (!fileName.contains(tmpFile))
                KIO::NetAccess::removeTempFile(tmpFile);
        } else {
            d->m_loadingMessages << KIO::NetAccess::lastErrorString();
            kDebug() << KIO::NetAccess::lastErrorString();
            updateState( ErrorState );
        }
        d->m_openMutex.unlock();
    }

    void WinMIDIObject::slotTrackStart()
    {
        for(int i=0; i<MIDI_CHANNELS; ++i)
            d->m_channelEvents[i] = 0;
        d->m_trackLabel.clear();
        d->m_currentTicks = d->m_engine->getCurrentTime();
        updateLoadProgress();
    }

    void WinMIDIObject::slotTrackEnd()
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
        d->m_currentTicks = d->m_engine->getCurrentTime();
        updateLoadProgress();
    }

    void WinMIDIObject::songFinished()
    {
        updateState( StoppedState );
        d->m_out->stop();
        d->m_lastBeat = 0;
        bool goNext = d->m_playlistIndex < d->m_playList.count()-1;
        emit finished();
        if (goNext && (d->m_playlistIndex < d->m_playList.count()-1))
            setCurrentSource(d->m_playList.at(d->m_playlistIndex+1));
    }

    void WinMIDIObject::updateState(State newState)
    {
        State oldState = d->m_state;
        if (oldState != newState) {
            d->m_state = newState;
            //kDebug() << "from:" << d->m_dbgState[oldState]
            //         << "to:" << d->m_dbgState[newState];
            emit stateChanged(newState, oldState);
        }
    }

    void WinMIDIObject::addSongPadding()
    {
        unsigned long tick = d->m_song.last()->getTick();
        tick += (d->m_beatMax * d->m_beatLength); // a full bar
        SongEvent* ev = new SongEvent(TICK_EVENT);
        ev->setTick(tick);
        d->m_song.append(ev);
        d->m_currentTicks = tick;
        updateLoadProgress();
    }

    void WinMIDIObject::emitShortEvSignals(qint64 ticks, const WinMIDIPacket& packet)
    {
        int value = 0;
        QString txt;
        int type = MEVT_EVENTTYPE(packet.dwPacket);
        int parm = MEVT_EVENTPARM(packet.dwPacket);
        if ((type & MEVT_NOP) != 0) {
            switch (parm) {
            case 0:
                emit tick(ticks);
                break;
            case 1:
                if (d->m_textUserEvents.contains(ticks)) {
                    if (d->m_codec == NULL)
                        txt = QString::fromAscii(d->m_textUserEvents[ticks]);
                    else
                        txt = d->m_codec->toUnicode(d->m_textUserEvents[ticks]);
                    txt.remove(QRegExp("[/\\\\]+"));
                    txt.remove(QRegExp("[\r\n]+"));
                    emit midiText(Song::Lyric, txt);
                }
                break;
            default:
                emit beat(packet.data[0], packet.data[1], packet.data[2]);
            }
            return;
        }
        if ((type & MEVT_TEMPO) != 0) {
            if (parm != 0) {
                qreal rt = 6.0e7f / parm;
                emit tempoChanged(rt);
            }
            return;
        }
        int status = packet.data[0] & MIDI_STATUS_MASK;
        int channel = packet.data[0] & MIDI_CHANNEL_MASK;
        switch (status) {
        case MIDI_STATUS_NOTEOFF:
            emit midiNoteOff(channel, packet.data[1], packet.data[2]);
            break;
        case MIDI_STATUS_NOTEON:
            emit midiNoteOn(channel, packet.data[1], packet.data[2]);
            break;
        case MIDI_STATUS_KEYPRESURE:
            emit midiKeyPressure(channel, packet.data[1], packet.data[2]);
            break;
        case MIDI_STATUS_CONTROLCHANGE:
            emit midiController(channel, packet.data[1], packet.data[2]);
            break;
        case MIDI_STATUS_PROGRAMCHANGE:
            emit midiProgram(channel, packet.data[1]);
            break;
        case MIDI_STATUS_CHANNELPRESSURE:
            emit midiChannelPressure(channel, packet.data[1]);
            break;
        case MIDI_STATUS_PITCHBEND:
            value = (packet.data[1] + packet.data[2] * 0x80) - 8192;
            emit midiPitchBend(channel, value);
            break;
        default:
            kDebug() << "status unknown:" << status;
        }
    }

    QVariant WinMIDIObject::songProperty(const QString& key)
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

    QVariant WinMIDIObject::channelProperty(int channel, const QString& key)
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

    void WinMIDIObject::sendInitialProgramChanges()
    {
        kDebug();
        for (int i = 0; i < MIDI_CHANNELS; ++i) {
            int patch(d->m_channelPatches[i]);
            d->m_out->sendInitialProgram(i, patch);
        }
    }
}

#include "winmidiobject.moc"
