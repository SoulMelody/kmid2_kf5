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

#ifndef INCLUDED_SONG_H
#define INCLUDED_SONG_H

#include "winmidioutput.h"
#include <QStringList>
#include <QMap>
#include <QVarLengthArray>

class QTextCodec;

namespace KMid {

    enum SongEventType {
        SHORT_EVENT = 0,
        TEMPO_EVENT,
        BEAT_EVENT,
        LYRIC_EVENT,
        SYSEX_EVENT,
        TICK_EVENT
    };

    class SongEvent
    {
    public:
        SongEvent(SongEventType t);
        SongEvent(SongEventType t, int);
        SongEvent(SongEventType t, int, int);
        SongEvent(SongEventType t, int, int, int);
        SongEvent(SongEventType t, const QByteArray& data);
        virtual ~SongEvent();
        SongEventType getType() const;
        qint64 getTick() const;
        const char* getData() const;
        int getDataLength() const;
        DWORD getPacket() const;
        void setTick(qint64 time);

    private:
        qint64 m_ticktime;
        SongEventType m_type;
        WinMIDIPacket m_packet;
        QByteArray m_data;
    };

    class Song : public QList<SongEvent*>
    {
    public:

        /**
         * For Karaoke files, there may be additional metadata
         * FileType: @KMIDI KARAOKE FILE
         * Version: @V0100
         * Information: @I<text>
         * Language: @L<lang>
         * Title: @T<title, author, ...>
         * Warning?: @W<bla bla bla>
         */
        enum TextType {
            Text = 1, Copyright = 2, TrackName = 3,
            InstrumentName = 4, Lyric = 5, Marker = 6, Cue = 7,
            KarFileType = 8, KarVersion = 9, KarInformation = 10,
            KarLanguage = 11, KarTitles = 12, KarWarnings = 13,
            FIRST_TYPE = Text, LAST_TYPE = KarWarnings
        };

        Song() : QList<SongEvent*>(),
            m_format(0),
            m_ntrks(0),
            m_division(0),
            m_codec(0)
        { }
        virtual ~Song();

        void clear();
        void sort();
        void setHeader(int format, int ntrks, int division);
        void setFileName(const QString& fileName);
        void addMetaData(TextType type, const QByteArray& text, const qint64 tick);
        void setTextCodec(QTextCodec *c);
        bool guessTextCodec();

        int getFormat() const { return m_format; }
        int getTracks() const { return m_ntrks; }
        int getDivision() const { return m_division; }
        QString getFileName() const { return m_fileName; }
        QTextCodec* getTextCodec() const { return m_codec; }
        QStringList getText(TextType type);
        QStringList getLyrics(qint64 time);

    private:
        void appendStringToList(QStringList &list, QString &s, TextType type = Text);
        QString decodeBytes(const QByteArray &ba);

        /**
         * Time-stamped data, like lyrics and similar meta data
         */
        typedef QMap<qint64, QByteArray> TimeStampedData;

        int m_format;
        int m_ntrks;
        int m_division;
        QTextCodec *m_codec;
        QString m_fileName;
        QMap<TextType, TimeStampedData> m_text;
    };

    typedef QListIterator<SongEvent*> SongIterator;

}

#endif /*INCLUDED_SONG_H*/
