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

#ifndef DUMMYMIDIOBJECT_H
#define DUMMYMIDIOBJECT_H

#include "midiobject.h"
#include <QObject>

namespace KMid {

    class DummyMIDIObject : public MIDIObject
    {
        Q_OBJECT

    public:
        explicit DummyMIDIObject(QObject *parent = 0);
        ~DummyMIDIObject();

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

    public Q_SLOTS:
        void setTickInterval(qint32 interval);
        void play();
        void pause();
        void stop();
        void seek(qint64 time);
        void clear();
        void setTimeSkew(qreal skew);
        void setTextEncoding(const QString& encoding);

    private:
        class DummyMIDIObjectPrivate;
        DummyMIDIObjectPrivate * const d;
    };

}

#endif // DUMMYMIDIOBJECT_H
