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

#ifndef MACMIDIOBJECT_H
#define MACMIDIOBJECT_H

#include "midiobject.h"
#include <QObject>
#include <AudioToolbox/MusicPlayer.h>
#include <CoreMIDI/MIDIServices.h>

namespace KMid {

    class MacMIDIOutput;

    enum UserDataType {
        UserEventTick,
        UserEventBeat,
        UserEventTimeSignature,
        UserEventTempo,
        UserEventText,
        UserEventFinished
    };

    enum TextType {
        Text = 1, Copyright = 2, TrackName = 3,
        InstrumentName = 4, Lyric = 5, Marker = 6, Cue = 7,
        KarFileType = 8, KarVersion = 9, KarInformation = 10,
        KarLanguage = 11, KarTitles = 12, KarWarnings = 13,
        FIRST_TYPE = Text, LAST_TYPE = KarWarnings
    };

    class MacMIDIObject : public MIDIObject
    {
        Q_OBJECT

    public:
        explicit MacMIDIObject(QObject *parent = 0);
        ~MacMIDIObject();

        void initialize(MacMIDIOutput *output);
        qint32 tickInterval() const;
        qint64 currentTime() const;
        State state() const;
        QString errorString() const;
        qint64 totalTime() const;
        qreal duration() const;
        qint64 remainingTime() const;
        QStringList metaData(const QString &key) const;
        QString currentSource() const;
        void setCurrentSource(const QString &source);
        QStringList queue() const;
        void setQueue(const QStringList &sources);
        void setQueue(const QList<QUrl> &urls);
        void enqueue(const QString &source);
        void enqueue(const QStringList &sources);
        void enqueue(const QList<QUrl> &urls);
        void clearQueue();
        qreal timeSkew();
        QString getTextEncoding() const;
        QStringList getLyrics(qint64 time) const;
        qreal currentTempo();
        bool channelUsed(int channel);
        int lowestMidiNote();
        int highestMidiNote();
        QString channelLabel(int channel);
        bool guessTextEncoding();
        QVariant songProperty(const QString& key);
        QVariant channelProperty(int channel, const QString& key);

        void openFile(const QString &fileName);
        void emitSignals(MIDIPacket* packet);
        void sendEvents(const MIDIPacketList *pktlist);
        void updateState(State newState);
        void analyzeTrack(int trackno);
        void analyzeSequence();
        void clearSequence();
        void addMetaData(TextType type, const QByteArray& text,
                const qint64 tick);
        QStringList getText(TextType type) const;
        bool guessTextCodec();
        void createFeedbackTrack();
        void sendUserSignal(UserDataType userType,
                MusicTimeStamp time);
        void sendInitialProgramChanges();

    public Q_SLOTS:
        void setTickInterval(qint32 interval);
        void play();
        void pause();
        void stop();
        void seek(qint64 time);
        void clear();
        void setTimeSkew(qreal skew);
        void setTextEncoding(const QString& encoding);
        void songFinished();

    Q_SIGNALS:
        void sequenceFinished();

    private:
        class MacMIDIObjectPrivate;
        MacMIDIObjectPrivate* const d;
    };

}

#endif // MACMIDIOBJECT_H
