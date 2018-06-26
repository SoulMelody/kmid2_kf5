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

#include "dummymidiobject.h"
#include <QStringList>

namespace KMid {

    class DummyMIDIObject::DummyMIDIObjectPrivate {
    public:
        DummyMIDIObjectPrivate()
        {}
        virtual ~DummyMIDIObjectPrivate()
        {}
    };

    DummyMIDIObject::DummyMIDIObject(QObject *parent) :
            MIDIObject(parent), d(new DummyMIDIObjectPrivate)
    { }


    DummyMIDIObject::~DummyMIDIObject()
    {
        delete d;
    }

    State DummyMIDIObject::state() const
    {
        return ErrorState;
    }

    qint32 DummyMIDIObject::tickInterval() const
    {
        return 0;
    }

    qint64 DummyMIDIObject::currentTime() const
    {
        return 0;
    }

    qreal DummyMIDIObject::duration() const
    {
        return 0;
    }

    qint64 DummyMIDIObject::remainingTime() const
    {
        return 0;
    }

    QString DummyMIDIObject::errorString() const
    {
        return QString();
    }

    QStringList DummyMIDIObject::metaData(const QString& /*key*/) const
    {
        /*if (key == "SMF_TEXT")
            //return d->m_song.getText(Song::Text);
        else if (key == "SMF_COPYRIGHT")
            //return d->m_song.getText(Song::Copyright);
        else if (key == "SMF_TRACKNAMES")
            //return d->m_song.getText(Song::TrackName);
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
            return d->m_song.getText(Song::KarWarnings);*/
        return QStringList();
    }

    qint64 DummyMIDIObject::totalTime() const
    {
        return 0;
    }

    QString DummyMIDIObject::currentSource() const
    {
        return QString();
    }

    void DummyMIDIObject::setCurrentSource(const QString& /*source*/)
    {
    }

    QStringList DummyMIDIObject::queue() const
    {
        return QStringList();
    }

    void DummyMIDIObject::setQueue(const QStringList& /*sources*/)
    {
    }

    void DummyMIDIObject::setQueue(const QList<QUrl>& /*urls*/ )
    {
    }

    void DummyMIDIObject::enqueue(const QString& /*source*/)
    {
    }

    void DummyMIDIObject::enqueue(const QStringList& /*sources*/)
    {
    }

    void DummyMIDIObject::enqueue(const QList<QUrl>& /*urls*/)
    {
    }

    void DummyMIDIObject::clearQueue()
    {
    }

    qreal DummyMIDIObject::timeSkew()
    {
        return 0;
    }

    /* SLOTS */

    void DummyMIDIObject::setTickInterval(qint32 /*interval*/)
    {
    }

    void DummyMIDIObject::play()
    {
    }

    void DummyMIDIObject::pause()
    {
    }

    void DummyMIDIObject::stop()
    {
    }

    void DummyMIDIObject::seek(qint64 /*time*/)
    {
    }

    void DummyMIDIObject::clear()
    {
    }

    void DummyMIDIObject::setTimeSkew(qreal /*skew*/)
    {
    }

    QString DummyMIDIObject::getTextEncoding() const
    {
        return QString();
    }

    void DummyMIDIObject::setTextEncoding(const QString& /*encoding*/)
    {
    }

    QStringList DummyMIDIObject::getLyrics(qint64 /*time*/) const
    {
        return QStringList();
    }

    qreal DummyMIDIObject::currentTempo()
    {
        return 0;
    }

    bool DummyMIDIObject::channelUsed(int /*channel*/)
    {
        return false;
    }

    int DummyMIDIObject::lowestMidiNote()
    {
        return 0;
    }

    int DummyMIDIObject::highestMidiNote()
    {
        return 0;
    }

    bool DummyMIDIObject::guessTextEncoding()
    {
        return false;
    }

    QString DummyMIDIObject::channelLabel(int /*channel*/)
    {
        return QString();
    }

    QVariant DummyMIDIObject::songProperty(const QString& /*key*/)
    {
        return QVariant();
    }

    QVariant DummyMIDIObject::channelProperty(int /*channel*/, const QString& /*key*/)
    {
        return QVariant();
    }

}

#include "dummymidiobject.moc"
