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

#ifndef WINMIDIOBJECT_H
#define WINMIDIOBJECT_H

#include "midiobject.h"
#include "song.h"
#include <QObject>

namespace KMid {

    union WinMIDIPacket;
    class WinMIDIOutput;

    class WinMIDIObject : public MIDIObject
    {
        Q_OBJECT

    public:
        explicit WinMIDIObject(QObject *parent = 0);
        ~WinMIDIObject();

        void initialize(WinMIDIOutput *output);
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
        void clearSong();
        qreal timeSkew();
        QString getTextEncoding() const;
        QStringList getLyrics(qint64 time) const;
        qreal currentTempo();
        bool channelUsed(int channel);
        int lowestMidiNote();
        int highestMidiNote();
        QString channelLabel(int channel);
        virtual bool guessTextEncoding();
        QVariant songProperty(const QString& key);
        QVariant channelProperty(int channel, const QString& key);
        void sendInitialProgramChanges();
        void emitShortEvSignals(qint64 ticks, const WinMIDIPacket& packet);

    public Q_SLOTS:
        void setTickInterval(qint32 interval);
        void play();
        void pause();
        void stop();
        void seek(qint64 time);
        void clear();
        void setTimeSkew(qreal skew);
        void setTextEncoding(const QString& encoding);

        /* SMF parsing slots */
        void headerEvent(int format, int ntrks, int division);
        void noteOnEvent(int chan, int pitch, int vol);
        void noteOffEvent(int chan, int pitch, int vol);
        void keyPressEvent(int chan, int pitch, int press);
        void ctlChangeEvent(int chan, int ctl, int value);
        void pitchBendEvent(int chan, int value);
        void programEvent(int chan, int patch);
        void chanPressEvent(int chan, int press);
        void sysexEvent(const QByteArray& data);
        void metaEvent(int type, const QByteArray& data);
        void tempoEvent(int tempo);
        void endOfTrackEvent();
        void errorHandler(const QString& errorStr);
        void timeSigEvent(int b0, int b1, int b2, int b3);
        void appendEvent(SongEvent *ev);
        void updateLoadProgress();
        void openFile(const QString &fileName);
        void songFinished();
        void updateState(State newState);
        void slotTrackStart();
        void slotTrackEnd();
        void addSongPadding();

    private:
        class WinMIDIObjectPrivate;
        WinMIDIObjectPrivate * const d;
    };

}

#endif // WINMIDIOBJECT_H
