/*
    KMid Mac OSX Backend
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

#include "macmidiobject.h"
#include "macmidioutput.h"
#include "midimapper.h"

#include <KDebug>
#include <KIO/NetAccess>
#include <KEncodingProber>
#include <QStringList>
#include <QMutex>
#include <QTextCodec>

#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>
#include <AudioToolbox/AudioToolbox.h>
#include <CoreMIDI/CoreMIDI.h>

namespace KMid {

    void MacMIDIReadProc( const MIDIPacketList *pktlist,
                          void *refCon, void *connRefCon )
    {
        MacMIDIObject *obj = NULL;
        if (refCon != NULL)
            obj = (MacMIDIObject *) refCon;
        if (obj != NULL)
            obj->sendEvents(pktlist);
        MIDIPacket *packet = (MIDIPacket *)pktlist->packet;
        for (unsigned int i = 0; i < pktlist->numPackets; ++i) {
            if (obj != NULL)
               obj->emitSignals(packet);
            packet = MIDIPacketNext(packet);
        }
    }

    void MacMusicSequenceUserCallback ( void *inClientData,
                                       MusicSequence inSequence,
                                       MusicTrack inTrack,
                                       MusicTimeStamp inEventTime,
                                       const MusicEventUserData *inEventData,
                                       MusicTimeStamp inStartSliceBeat,
                                       MusicTimeStamp inEndSliceBeat )
    {
        MacMIDIObject *obj = NULL;
        UserDataType userType = (UserDataType) inEventData->data[0];
        if (inClientData != NULL)
            obj = (MacMIDIObject *) inClientData;
        if (obj != NULL)
            obj->sendUserSignal(userType, inEventTime);
    }

    typedef QMap<qint64, QByteArray> TimeStampedData;

    struct TimeSignatureRec {
        int num;
        int den;
    };

    struct BeatRec {
        int bar;
        int beat;
        int max;
    };

    class MacMIDIObject::MacMIDIObjectPrivate {
    public:
        MacMIDIObjectPrivate() :
            m_player(0),
            m_sequence(0),
            m_endpoint(0),
            m_output(0),
            m_codec(0),
            m_state(BufferingState),
            m_playlistIndex(-1),
            m_tempoFactor(1.0),
            m_lowestMidiNote(127),
            m_highestMidiNote(0),
            m_sequenceLength(0),
            m_duration(0),
            m_division(0),
            m_songEmpty(true),
            m_tickInterval(0),
            m_numtracks(0),
            m_barCount(0)
        { }

        virtual ~MacMIDIObjectPrivate()
        {
            OSStatus result;
            result = DisposeMusicPlayer(m_player);
            if (result != noErr)
                kDebug() << "DisposeMusicPlayer() err:" << result;
            if (m_sequence != NULL) {
                result = DisposeMusicSequence(m_sequence);
                if (result != noErr)
                    kDebug() << "DisposeMusicSequence() err:" << result;
            }
            if (m_endpoint != NULL) {
                result = MIDIEndpointDispose(m_endpoint);
                if (result != noErr)
                    kDebug() << "MIDIEndpointDispose() err:" << result;
            }
        }

        MusicPlayer m_player;
        MusicSequence m_sequence;
        MIDIEndpointRef m_endpoint;
        MacMIDIOutput *m_output;
        QTextCodec *m_codec;
        State m_state;
        int m_playlistIndex;
        qreal m_tempoFactor;
        QStringList m_loadingMessages;
        QStringList m_playList;
        QString m_encoding;
        int m_lowestMidiNote;
        int m_highestMidiNote;
        bool m_channelUsed[MIDI_CHANNELS];
        QByteArray m_channelLabel[MIDI_CHANNELS];
        int m_channelPatches[MIDI_CHANNELS];
        QMutex m_openMutex;
        MusicTimeStamp m_sequenceLength;
        qreal m_duration;
        int m_division;
        bool m_songEmpty;
        qint32 m_tickInterval;
        int m_numtracks;
        int m_barCount;
        QMap<TextType, TimeStampedData> m_text;
        QMap<MusicTimeStamp, QByteArray> m_textUserEvents;
        QMap<MusicTimeStamp, qreal> m_tempoUserEvents;
        QMap<MusicTimeStamp, TimeSignatureRec> m_timesigUserEvents;
        QMap<MusicTimeStamp, BeatRec> m_beatUserEvents;
    };

    MacMIDIObject::MacMIDIObject(QObject *parent) :
            MIDIObject(parent), d(new MacMIDIObjectPrivate)
    {
        OSStatus result;
        result = NewMusicPlayer(&d->m_player);
        if (result != noErr)
            kDebug() << "NewMusicPlayer() err:" << result;
        connect ( this, SIGNAL(sequenceFinished()), SLOT(songFinished()), Qt::QueuedConnection);
    }

    MacMIDIObject::~MacMIDIObject()
    {
        delete d;
    }

    void MacMIDIObject::initialize(MacMIDIOutput *output)
    {
        OSStatus result;
        d->m_output = output;
        result = MIDIDestinationCreate ( output->client(),
                    CFSTR("KMid Loopback"), MacMIDIReadProc,
                    (void*) this, &d->m_endpoint );
        if (result != noErr)
            kDebug() << "MIDIDestinationCreate() err:" << result;
        else {
            result = MIDIObjectSetIntegerProperty( d->m_endpoint,
                        kMIDIPropertyPrivate, 1 );
            if (result != noErr)
                kDebug() << "MIDIObjectSetIntegerProperty() err:" << result;
        }
    }

    void MacMIDIObject::emitSignals(MIDIPacket* packet)
    {
        int value = 0;
        int status = packet->data[0] & 0xf0;
        int channel = packet->data[0] & 0x0f;
        QByteArray data;
        switch (status) {
        case MIDI_STATUS_NOTEOFF:
            emit midiNoteOff(channel, packet->data[1], packet->data[2]);
            break;
        case MIDI_STATUS_NOTEON:
            emit midiNoteOn(channel, packet->data[1], packet->data[2]);
            break;
        case MIDI_STATUS_KEYPRESURE:
            emit midiKeyPressure(channel, packet->data[1], packet->data[2]);
            break;
        case MIDI_STATUS_CONTROLCHANGE:
            emit midiController(channel, packet->data[1], packet->data[2]);
            break;
        case MIDI_STATUS_PROGRAMCHANGE:
            emit midiProgram(channel, packet->data[1]);
            break;
        case MIDI_STATUS_CHANNELPRESSURE:
            emit midiChannelPressure(channel, packet->data[1]);
            break;
        case MIDI_STATUS_PITCHBEND:
            value = (packet->data[1] + packet->data[2] * 0x80) - 8192;
            emit midiPitchBend(channel, value);
            break;
        case MIDI_STATUS_SYSEX:
            data = QByteArray((const char *)packet->data, packet->length);
            emit midiSysex(data);
            break;
        default:
            kDebug() << "status?" << status;
        }
    }

    void MacMIDIObject::sendEvents(const MIDIPacketList *pktlist)
    {
        MacMIDIOutput *out = d->m_output;
        if (out != NULL)
           out->sendEvents(pktlist);
    }

    State MacMIDIObject::state() const
    {
        return d->m_state;
    }

    qint32 MacMIDIObject::tickInterval() const
    {
        return d->m_tickInterval;
    }

    qint64 MacMIDIObject::currentTime() const
    {
        OSStatus result = noErr;
        MusicTimeStamp curTime;
        result = MusicPlayerGetTime( d->m_player, &curTime );
        if (result == noErr)
            return curTime * d->m_division;
        else
            kDebug() << "MusicPlayerGetTime() err:" << result;
        return 0;
    }

    qreal MacMIDIObject::duration() const
    {
        return d->m_duration;
    }

    qint64 MacMIDIObject::remainingTime() const
    {
        if (d->m_songEmpty)
            return 0;
        return totalTime() - currentTime();
    }

    QString MacMIDIObject::errorString() const
    {
        return d->m_loadingMessages.join(QString(QChar::LineSeparator));
    }

    QStringList MacMIDIObject::metaData(const QString & key) const
    {
        if (key == "SMF_TEXT")
            return getText(Text);
        else if (key == "SMF_COPYRIGHT")
            return getText(Copyright);
        else if (key == "SMF_TRACKNAMES")
            return getText(TrackName);
        else if (key == "SMF_INSTRUMENTNAMES")
            return getText(InstrumentName);
        else if (key == "SMF_LYRICS")
            return getText(Lyric);
        else if (key == "SMF_MARKERS")
            return getText(Marker);
        else if (key == "SMF_CUES")
            return getText(Cue);
        else if (key == "KAR_FILETYPE")
            return getText(KarFileType);
        else if (key == "KAR_VERSION")
            return getText(KarVersion);
        else if (key == "KAR_INFORMATION")
            return getText(KarInformation);
        else if (key == "KAR_LANGUAGE")
            return getText(KarLanguage);
        else if (key == "KAR_TITLES")
            return getText(KarTitles);
        else if (key == "KAR_WARNINGS")
            return getText(KarWarnings);
        return QStringList();
    }

    qint64 MacMIDIObject::totalTime() const
    {
        if (d->m_songEmpty)
            return 0;
        return d->m_sequenceLength * d->m_division;
    }

    QString MacMIDIObject::currentSource() const
    {
        if ( !d->m_songEmpty &&
             d->m_playlistIndex >=0 &&
             d->m_playlistIndex < d->m_playList.size() )
            return d->m_playList.at(d->m_playlistIndex);
        return QString();
    }

    void MacMIDIObject::setCurrentSource(const QString &source)
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

    QStringList MacMIDIObject::queue() const
    {
        return d->m_playList;
    }

    void MacMIDIObject::setQueue(const QStringList &sources)
    {
        d->m_playList = sources;
    }

    void MacMIDIObject::setQueue(const QList<QUrl> &urls)
    {
        d->m_playList.clear();
        enqueue(urls);
    }

    void MacMIDIObject::enqueue(const QString &source)
    {
        d->m_playList.append( source );
    }

    void MacMIDIObject::enqueue(const QStringList &sources)
    {
        d->m_playList += sources;
    }

    void MacMIDIObject::enqueue(const QList<QUrl> &urls)
    {
        foreach(const QUrl &u, urls) {
            d->m_playList.append( u.toString() );
        }
    }

    void MacMIDIObject::clearQueue()
    {
        d->m_playList.clear();
        d->m_playlistIndex = -1;
    }

    qreal MacMIDIObject::timeSkew()
    {
        return d->m_tempoFactor;
    }

    /* SLOTS */

    void MacMIDIObject::setTickInterval(qint32 interval)
    {
        d->m_tickInterval = interval;
    }

    void MacMIDIObject::play()
    {
        OSStatus result;
        if ( !d->m_songEmpty &&
             (d->m_state == PausedState || d->m_state == StoppedState) ) {
            if (currentTime() == 0) {
                d->m_output->sendResetMessage();
                d->m_output->resetControllers();
                sendInitialProgramChanges();
            }
            result = MusicPlayerStart ( d->m_player );
            if (result == noErr)
                updateState( PlayingState );
            else
                kDebug() << "MusicPlayerStart() err:" << result;
        }
    }

    void MacMIDIObject::pause()
    {
        OSStatus result = noErr;
        if (d->m_state == PlayingState) {
            result = MusicPlayerStop ( d->m_player );
            if (result == noErr)
                updateState( PausedState );
            else
                kDebug() << "MusicPlayerStop() err:" << result;
        }
    }

    void MacMIDIObject::stop()
    {
        OSStatus result = noErr;
        if ((d->m_state == PlayingState) || (d->m_state == PausedState)) {
            result = MusicPlayerStop ( d->m_player );
            if (result == noErr) {
                updateState( StoppedState );
                d->m_output->allNotesOff();
                result = MusicPlayerSetTime ( d->m_player, 0 );
                if (result != noErr)
                    kDebug() << "MusicPlayerSetTime() err:" << result;
            } else
                kDebug() << "MusicPlayerStop() err:" << result;
        }
    }

    void MacMIDIObject::seek(qint64 time)
    {
        OSStatus result = noErr;
        MusicTimeStamp ts = double(time) / d->m_division;
        if ( (ts >= 0) && (ts < d->m_sequenceLength ) ) {
            bool running = (d->m_state == PlayingState);
            if (running) {
                result = MusicPlayerStop ( d->m_player );
                if (result == noErr)
                    updateState( PausedState );
                else
                    kDebug() << "MusicPlayerStop() err:" << result;
            }

            result = MusicPlayerSetTime ( d->m_player, ts );
            if (result != noErr)
                kDebug() << "MusicPlayerSetTime() err:" << result;

            if (running) {
                result = MusicPlayerStart ( d->m_player );
                if (result == noErr)
                    updateState( PlayingState );
                else
                    kDebug() << "MusicPlayerStart() err:" << result;
            }
        }
    }

    void MacMIDIObject::clear()
    {
        clearSequence();
        clearQueue();
    }

    void MacMIDIObject::setTimeSkew(qreal skew)
    {
        OSStatus result;
        if (d->m_tempoFactor != skew) {
            d->m_tempoFactor = skew;
            result = MusicPlayerSetPlayRateScalar ( d->m_player, skew );
            if (result != noErr)
                kDebug() << "MusicPlayerSetPlayRateScalar() err:" << result;
        }
    }

    void MacMIDIObject::clearSequence()
    {
        OSStatus result = noErr;
        MusicSequence oldsequence = d->m_sequence;
        if (d->m_sequence != 0) {
            result = MusicSequenceSetUserCallback( d->m_sequence, NULL, NULL );
            if (result != noErr)
                kDebug() << "MusicSequenceSetUserCallback() err:" << result;
        }
        d->m_sequence = 0;

        result = NewMusicSequence(&d->m_sequence);
        if (result != noErr)
            kDebug() << "NewMusicSequence() err:" << result;

        result = MusicSequenceSetUserCallback( d->m_sequence,
                    MacMusicSequenceUserCallback, (void *) this );
        if (result != noErr)
            kDebug() << "MusicSequenceSetUserCallback() err:" << result;

        result = MusicPlayerSetSequence (d->m_player, d->m_sequence);
        if (result != noErr)
            kDebug() << "MusicPlayerSetSequence() err:" << result;

        if (oldsequence != 0) {
            result = DisposeMusicSequence(oldsequence);
            if (result != noErr)
                kDebug() << "DisposeMusicSequence() err:" << result;
        }

        d->m_text.clear();
        d->m_songEmpty = true;
    }

    void MacMIDIObject::openFile(const QString &fileName)
    {
        QString tmpFile;
        QByteArray fb;
        d->m_openMutex.lock();
        if (KIO::NetAccess::download(fileName, tmpFile, 0)) {
            updateState( LoadingState );
            clearSequence();
            d->m_loadingMessages.clear();
            fb = tmpFile.toLocal8Bit();
            OSStatus result = noErr;
            MusicSequenceLoadFlags loadFlags = 0; //kMusicSequenceLoadSMF_ChannelsToTracks;
            CFURLRef url = CFURLCreateFromFileSystemRepresentation (
                kCFAllocatorDefault, (UInt8*)fb.data(), fb.size(), false);
            result = MusicSequenceFileLoad (d->m_sequence, url, 0, loadFlags);
            if (result == noErr) {
                analyzeSequence();
                if (!d->m_songEmpty) {
                    createFeedbackTrack();
                    result = MusicSequenceSetMIDIEndpoint (d->m_sequence,
                                d->m_endpoint);
                    if (result != noErr)
                        kDebug() << "MusicSequenceSetMIDIEndpoint() err:"
                                 << result;
                    result = MusicPlayerSetTime (d->m_player, 0);
                    if (result != noErr)
                        kDebug() << "MusicPlayerSetTime() err:" << result;
                    result = MusicPlayerPreroll ( d->m_player );
                    if (result != noErr)
                        kDebug() << "MusicPlayerPreroll() err:" << result;
                }
                updateState ( StoppedState );
                emit currentSourceChanged(fileName);
            } else {
                kDebug() << "MusicSequenceFileLoad() err:" << result;
                clearSequence();
                updateState( ErrorState );
            }
            CFRelease(url);
            KIO::NetAccess::removeTempFile(tmpFile);
        } else {
            d->m_loadingMessages << KIO::NetAccess::lastErrorString();
            updateState( ErrorState );
        }
        d->m_openMutex.unlock();
    }

    void MacMIDIObject::addMetaData( TextType type,
                                     const QByteArray& text,
                                     const qint64 tick )
    {
        if ( (type >= FIRST_TYPE) && (type <= Cue) ) {
            TextType t = type;
            if ((text.length() > 0) && (text[0] == '%'))
                return; // ignored
            if ((text.length() > 1) && (text[0] == '@')) {
                switch(text[1]) {
                case 'K':
                    t = KarFileType;
                    break;
                case 'V':
                    t = KarVersion;
                    break;
                case 'I':
                    t = KarInformation;
                    break;
                case 'L':
                    t = KarLanguage;
                    break;
                case 'T':
                    t = KarTitles;
                    break;
                case 'W':
                    t = KarWarnings;
                    break;
                }
            }
            d->m_text[t][tick].append(text);
        }
    }

    void appendStringToList(QStringList &list, QString &s, TextType type)
    {
        if (type == Text || type >= KarFileType)
            s.replace(QRegExp("@[IKLTVW]"), QString(QChar::LineSeparator));
        if (type == Text || type == Lyric)
            s.replace(QRegExp("[/\\\\]+"), QString(QChar::LineSeparator));
        s.replace(QRegExp("[\r\n]+"), QString(QChar::LineSeparator));
        list.append(s);
    }

    QStringList MacMIDIObject::getText(TextType type) const
    {
        QString s;
        QStringList list;
        if ( (type >= FIRST_TYPE) && (type <= LAST_TYPE) ) {
            foreach(const QByteArray &a, d->m_text[type]) {
                if (d->m_codec == NULL )
                    s = QString::fromAscii(a);
                else
                    s = d->m_codec->toUnicode(a);
                appendStringToList(list, s, type);
            }
        }
        return list;
    }

    QStringList MacMIDIObject::getLyrics(qint64 time) const
    {
        TextType t;
        QStringList list;
        TimeStampedData::const_iterator it, end;
        if (d->m_text[Lyric].empty()) {
            t = Text;
            it = d->m_text[Text].constBegin();
            if (d->m_text[Text].empty())
                end = d->m_text[Text].constEnd();
            else {
                const TimeStampedData td = d->m_text[Text];
                end = td.upperBound(time);
            }
        } else {
            t = Lyric;
            const TimeStampedData td = d->m_text[Lyric];
            it = td.constBegin();
            end = td.upperBound(time);
        }
        for (; it != end; ++it ) {
            QString s;
            if (d->m_codec == NULL )
                s = QString::fromAscii(it.value());
            else
                s = d->m_codec->toUnicode(it.value());
            appendStringToList(list, s, t);
        }
        return list;
    }

    void MacMIDIObject::analyzeTrack(int trackno)
    {
        OSStatus result = noErr;
        MusicTrack track;
        MusicEventIterator iter;
        Boolean hasCurrentEvent;
        QByteArray trackLabel;
        int channelEvents[MIDI_CHANNELS];
        for(int i=0; i<MIDI_CHANNELS; ++i)
            channelEvents[i] = 0;
        //kDebug() << "Track:" << trackno;
        if (trackno < 0) {
            MusicSequenceGetTempoTrack(d->m_sequence, &track);
            SInt16 timeResolution;
            UInt32 tSize = sizeof(timeResolution);
            result = MusicTrackGetProperty(track,
                       kSequenceTrackProperty_TimeResolution,
                       &timeResolution, &tSize );
            d->m_division = timeResolution;
            setTickInterval(d->m_division / 6);
            //CAShow(track);
        } else {
            MusicTimeStamp trackLength;
            UInt32 propsize = sizeof(MusicTimeStamp);
            result = MusicSequenceGetIndTrack(d->m_sequence, trackno, &track);
            result = MusicTrackGetProperty(track,
                        kSequenceTrackProperty_TrackLength,
                        &trackLength, &propsize);
            if (trackLength > d->m_sequenceLength)
                d->m_sequenceLength = trackLength;
        }
        NewMusicEventIterator(track, &iter);
        MusicEventIteratorHasCurrentEvent (iter, &hasCurrentEvent);
        while (hasCurrentEvent) {
            MusicTimeStamp timeStamp;
            MusicEventType eventType;
            const void *dataPtr;
            UInt32 dataLen;
            MusicEventIteratorGetEventInfo ( iter, &timeStamp, &eventType,
                &dataPtr, &dataLen );
            switch (eventType) {
            case kMusicEventType_ExtendedTempo: {
                ExtendedTempoEvent* tmpp = (ExtendedTempoEvent *) dataPtr;
                d->m_tempoUserEvents.insert(timeStamp, tmpp->bpm);
                break;
            }
            case kMusicEventType_Meta: {
                MIDIMetaEvent* metap = (MIDIMetaEvent*) dataPtr;
                if ( metap->metaEventType == 0x58 ) {
                    TimeSignatureRec ts;
                    ts.num = metap->data[0];
                    ts.den =  ::pow(2, metap->data[1]);
                    d->m_timesigUserEvents.insert(timeStamp, ts);
                } else
                if ( (metap->metaEventType >= FIRST_TYPE) &&
                     (metap->metaEventType <= Cue) ) {
                    QByteArray data( reinterpret_cast<const char *>(metap->data),
                                     metap->dataLength );
                    qint64 tick = timeStamp * d->m_division;
                    addMetaData((TextType)metap->metaEventType, data, tick);
                    if ( ( metap->metaEventType == Lyric ||
                           metap->metaEventType == Text ) &&
                         (data.length() > 0) &&
                         (data[0] != '@') && (data[0] != '%') ) {
                        data.truncate(32);
                        d->m_textUserEvents[timeStamp] += data;
                    }

                    if ( (metap->metaEventType == TrackName ||
                          metap->metaEventType == InstrumentName ) &&
                          trackLabel.isEmpty() )
                        trackLabel = data;
                }
                break;
            }
            case kMusicEventType_MIDINoteMessage: {
                MIDINoteMessage* notep = (MIDINoteMessage*) dataPtr;
                d->m_channelUsed[notep->channel] = true;
                if (d->m_lowestMidiNote > notep->note)
                    d->m_lowestMidiNote = notep->note;
                if (d->m_highestMidiNote < notep->note)
                    d->m_highestMidiNote = notep->note;
                channelEvents[notep->channel]++;
                break;
            }
            case kMusicEventType_MIDIChannelMessage: {
                MIDIChannelMessage* msgp = (MIDIChannelMessage*) dataPtr;
                quint8 channel = msgp->status & 0x0f;
                quint8 type = msgp->status & 0xf0;
                if ( (type == MIDI_STATUS_PROGRAMCHANGE) &&
                     (d->m_channelPatches[channel] < 0) ) {
                    d->m_channelPatches[channel] = msgp->data1;
                }
                d->m_channelUsed[channel] = true;
                channelEvents[channel]++;
                break;
            }
            default:
                //kDebug() << "Time:" << timeStamp << "Type:" << eventType << "Len:" << dataLen;
                break;
            }
            MusicEventIteratorNextEvent (iter);
            MusicEventIteratorHasCurrentEvent (iter, &hasCurrentEvent);
        }
        DisposeMusicEventIterator(iter);
        if (!trackLabel.isEmpty()) {
            int max = 0;
            int chan = -1;
            for(int i=0; i<MIDI_CHANNELS; ++i)
                if (channelEvents[i] > max) {
                    max = channelEvents[i];
                    chan = i;
                }
            if (chan >= 0 && chan < MIDI_CHANNELS)
                d->m_channelLabel[chan] = trackLabel;
        }
    }

    void MacMIDIObject::analyzeSequence()
    {
        OSStatus result = noErr;
        UInt32 ntracks;
        Float64 lengthSeconds;
        d->m_numtracks = 0;
        d->m_barCount = 0;
        d->m_duration = 0;
        d->m_sequenceLength = 0;
        d->m_lowestMidiNote = 127;
        d->m_highestMidiNote = 0;
        d->m_beatUserEvents.clear();
        d->m_timesigUserEvents.clear();
        d->m_tempoUserEvents.clear();
        d->m_textUserEvents.clear();
        for(int i=0; i<MIDI_CHANNELS; ++i) {
            d->m_channelUsed[i] = false;
            d->m_channelLabel[i].clear();
            d->m_channelPatches[i] = -1;
        }

        result = MusicSequenceGetTrackCount (d->m_sequence, &ntracks);
        if (result != noErr)
            kDebug() << "MusicSequenceGetTrackCount() err:" << result;
        d->m_numtracks = result;
        analyzeTrack(-1); // tempoTrack
        for (UInt32 i = 0; i < ntracks; ++i)
            analyzeTrack(i);
        result = MusicSequenceGetSecondsForBeats ( d->m_sequence,
                    d->m_sequenceLength, &lengthSeconds );
        if (result != noErr)
            kDebug() << "MusicSequenceGetSecondsForBeats() err:" << result;
        d->m_duration = lengthSeconds;
        d->m_songEmpty = !(d->m_sequenceLength > 0);
    }

    void MacMIDIObject::songFinished()
    {
        stop();
        bool goNext = (d->m_playlistIndex < d->m_playList.count()-1);
        emit finished();
        if (goNext && (d->m_playlistIndex < d->m_playList.count()-1))
            setCurrentSource(d->m_playList.at(d->m_playlistIndex+1));
    }

    void MacMIDIObject::updateState(State newState)
    {
        State oldState = d->m_state;
        if (oldState != newState) {
            d->m_state = newState;
            emit stateChanged(newState, oldState);
        }
    }

    QString MacMIDIObject::getTextEncoding() const
    {
        return d->m_encoding;
    }

    void MacMIDIObject::setTextEncoding(const QString& encoding)
    {
        if (encoding != d->m_encoding) {
            if (encoding.isEmpty())
                d->m_codec = NULL;
            else
                d->m_codec = QTextCodec::codecForName(encoding.toLatin1());
            d->m_encoding = encoding;
        }
    }

    qreal MacMIDIObject::currentTempo()
    {
        OSStatus result;
        qreal tempo = 0.0;
        MusicTimeStamp curTime = 0;
        result = MusicPlayerGetTime( d->m_player, &curTime );
        if (result != noErr)
            kDebug() << "MusicPlayerGetTime() err:" << result;
        QMapIterator<MusicTimeStamp, qreal> iter(d->m_tempoUserEvents);
        while (iter.hasNext()) {
            iter.next();
            if (iter.key() < curTime)
                tempo = iter.value();
            else
                break;
        }
        return tempo * d->m_tempoFactor; // real tempo
    }

    bool MacMIDIObject::channelUsed(int channel)
    {
        return d->m_channelUsed[channel];
    }

    int MacMIDIObject::lowestMidiNote()
    {
        return d->m_lowestMidiNote;
    }

    int MacMIDIObject::highestMidiNote()
    {
        return d->m_highestMidiNote;
    }

    bool MacMIDIObject::guessTextCodec()
    {
        KEncodingProber prober;
        TimeStampedData::const_iterator it, end;
        if (d->m_text[Lyric].empty()) {
            it = d->m_text[Text].constBegin();
            end = d->m_text[Text].constEnd();
        } else {
            it = d->m_text[Lyric].constBegin();
            end = d->m_text[Lyric].constEnd();
        }
        if (it == end)
            return false;
        for (; it != end; ++it )
            prober.feed( it.value() );
        if ( prober.confidence() > 0.6 ) {
            QTextCodec *codec = QTextCodec::codecForName(prober.encodingName());
            if (codec == NULL)
                kWarning() << "Unsupported encoding detected:"
                           << prober.encodingName();
            else {
                d->m_codec = codec;
                return true;
            }
        }
        return false;
    }

    bool MacMIDIObject::guessTextEncoding()
    {
        bool res = guessTextCodec();
        if (res && d->m_codec != NULL)
            d->m_encoding = QString(d->m_codec->name());
        return res;
    }

    QString MacMIDIObject::channelLabel(int channel)
    {
        if (channel >= 0 && channel < MIDI_CHANNELS) {
            if (d->m_codec == NULL)
                return QString::fromAscii(d->m_channelLabel[channel]);
            else
                return d->m_codec->toUnicode(d->m_channelLabel[channel]);
        }
        return QString();
    }

    void MacMIDIObject::createFeedbackTrack()
    {
        OSStatus result;
        MusicTrack userTrack;
        MusicEventUserData userEv;
        userEv.length = 1;
        result =  MusicSequenceNewTrack ( d->m_sequence, &userTrack );
        if ( result != noErr )
            kDebug() << "MusicSequenceNewTrack() err:" << result;
        else {
            QMapIterator<MusicTimeStamp, QByteArray> iter1(d->m_textUserEvents);
            while (iter1.hasNext()) {
                iter1.next();
                userEv.data[0] = UserEventText;
                result = MusicTrackNewUserEvent ( userTrack, iter1.key(), &userEv );
                if (result != noErr)
                    kDebug() << "MusicTrackNewUserEvent() err:" << result;
            }

            QMapIterator<MusicTimeStamp, qreal> iter2(d->m_tempoUserEvents);
            while (iter2.hasNext()) {
                iter2.next();
                userEv.data[0] = UserEventTempo;
                result = MusicTrackNewUserEvent ( userTrack, iter2.key(), &userEv );
                if (result != noErr)
                    kDebug() << "MusicTrackNewUserEvent() err:" << result;
            }

            QMapIterator<MusicTimeStamp, TimeSignatureRec> iter3(d->m_timesigUserEvents);
            while (iter3.hasNext()) {
                iter3.next();
                userEv.data[0] = UserEventTimeSignature;
                result = MusicTrackNewUserEvent ( userTrack, iter3.key(), &userEv );
                if (result != noErr)
                    kDebug() << "MusicTrackNewUserEvent() err:" << result;
            }

            MusicTimeStamp bTime = 0;
            TimeSignatureRec ts;
            float blen = 1.0;
            ts.num = 4;
            ts.den = 4;
            if (d->m_timesigUserEvents.contains(bTime)) {
                ts = d->m_timesigUserEvents[bTime];
                blen = 4.0 / ts.den;
            }
            //kDebug() << "TS initial:" << ts.num << ts.den;
            BeatRec br;
            br.bar = 1;
            br.beat = 1;
            br.max = ts.num;
            while ( bTime < d->m_sequenceLength ) {
                d->m_barCount = br.bar;
                d->m_beatUserEvents.insert(bTime, br);
                userEv.data[0] = UserEventBeat;
                result = MusicTrackNewUserEvent ( userTrack, bTime, &userEv );
                if (result != noErr)
                    kDebug() << "MusicTrackNewUserEvent() err:" << result;
                bTime += blen; // next beat
                br.beat++;
                if (br.beat > br.max) {
                    br.beat = 1;
                    br.bar++;
                }
                if (d->m_timesigUserEvents.contains(bTime)) {
                    ts = d->m_timesigUserEvents[bTime];
                    //kDebug() << "TS change:" << bTime << ts.num << ts.den;
                    br.max = ts.num;
                    blen = 4.0 / ts.den;
                }
            }
            bTime = 0;
            userEv.data[0] = UserEventTick;
            double interval = double(d->m_tickInterval) / d->m_division;
            while ( bTime < d->m_sequenceLength ) {
                result = MusicTrackNewUserEvent ( userTrack, bTime, &userEv );
                if (result != noErr)
                    kDebug() << "MusicTrackNewUserEvent() err:" << result;
                bTime += interval;
            }

            userEv.data[0] = UserEventFinished;
            result = MusicTrackNewUserEvent ( userTrack,
                d->m_sequenceLength + ts.num, &userEv );
        }
    }

    void MacMIDIObject::sendUserSignal( UserDataType userType,
                                        MusicTimeStamp time )
    {
        QString txt;
        switch (userType) {
        case UserEventTick:
            emit tick(time * d->m_division);
            break;
        case UserEventBeat:
            if (d->m_beatUserEvents.contains(time)) {
                BeatRec br = d->m_beatUserEvents[time];
                emit beat( br.bar, br.beat, br.max );
            }
            break;
        case UserEventTimeSignature:
            if (d->m_timesigUserEvents.contains(time)) {
                TimeSignatureRec ts = d->m_timesigUserEvents[time];
                emit timeSignatureChanged(ts.num, ts.den);
            }
            break;
        case UserEventTempo:
            if (d->m_tempoUserEvents.contains(time)) {
                qreal tempo = d->m_tempoUserEvents[time];
                emit tempoChanged(tempo);
            }
            break;
        case UserEventText:
            if (d->m_textUserEvents.contains(time)) {
                if (d->m_codec == NULL)
                    txt = QString::fromAscii(d->m_textUserEvents[time]);
                else
                    txt = d->m_codec->toUnicode(d->m_textUserEvents[time]);
                txt.remove(QRegExp("[/\\\\]+"));
                txt.remove(QRegExp("[\r\n]+"));
                emit midiText(Lyric, txt);
            }
            break;
        case UserEventFinished:
            emit sequenceFinished();
            break;
        }
    }

    QVariant MacMIDIObject::songProperty(const QString& key)
    {
        if (key == QLatin1String("SMF_FORMAT"))
            return QVariant(1);
        else if (key == QLatin1String("SMF_TRACKS"))
            return QVariant(d->m_numtracks);
        else if (key == QLatin1String("SMF_DIVISION"))
            return QVariant(d->m_division);
        else if (key == QLatin1String("NUM_BARS"))
            return QVariant(d->m_barCount);
        else if (key == QLatin1String("NUM_BEATS"))
            return QVariant(trunc(d->m_sequenceLength));
        return QVariant();
    }

    QVariant MacMIDIObject::channelProperty(int channel, const QString& key)
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

    void MacMIDIObject::sendInitialProgramChanges()
    {
        for (int i = 0; i < MIDI_CHANNELS; ++i) {
            int patch(d->m_channelPatches[i]);
            d->m_output->sendInitialProgram(i, patch);
        }
    }

}

#include "macmidiobject.moc"
