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

#include "song.h"
#include <QTextDecoder>
#include <KEncodingProber>
#include <KGlobal>
#include <KCharsets>
#include <KDebug>

namespace KMid {

    static inline bool eventLessThan(const SequencerEvent* s1, const SequencerEvent* s2)
    {
        return s1->getTick() < s2->getTick();
    }

    Song::~Song()
    {
        clear();
    }

    void Song::sort()
    {
        qStableSort(begin(), end(), eventLessThan);
    }

    void Song::clear()
    {
        while (!isEmpty())
            delete takeFirst();
        m_fileName.clear();
        m_text.clear();
        m_format = 0;
        m_ntrks = 0;
        m_division = 0;
    }

    void Song::setHeader(int format, int ntrks, int division)
    {
        m_format = format;
        m_ntrks = ntrks;
        m_division = division;
    }

    void Song::setFileName(const QString& fileName)
    {
        m_fileName = fileName;
    }

    void Song::addMetaData(TextType type, const QByteArray& text, const qint64 tick)
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
            m_text[t][tick].append(text);
        }
    }

    void Song::appendStringToList(QStringList &list, QString &s, TextType type)
    {
        if (type == Text || type >= KarFileType)
            s.replace(QRegExp("@[IKLTVW]"), QString(QChar::LineSeparator));
        if (type == Text || type == Lyric)
            s.replace(QRegExp("[/\\\\]+"), QString(QChar::LineSeparator));
        s.replace(QRegExp("[\r\n]+"), QString(QChar::LineSeparator));
        list.append(s);
    }

    void Song::setTextCodec(QTextCodec *c)
    {
        m_codec = c;
    }

    QString Song::decodeBytes(const QByteArray &ba)
    {
        if (m_codec == NULL )
            return QString::fromAscii(ba);
        return m_codec->toUnicode(ba);
    }

    QStringList Song::getText(TextType type)
    {
        QStringList list;
        if ( (type >= FIRST_TYPE) && (type <= LAST_TYPE) ) {
            foreach(const QByteArray &a, m_text[type]) {
                QString s = decodeBytes(a);
                appendStringToList(list, s, type);
            }
        }
        return list;
    }

    QStringList Song::getLyrics(qint64 time)
    {
        TextType t;
        QStringList list;
        TimeStampedData::const_iterator it, end;
        if (m_text[Lyric].empty()) {
            t = Text;
            it = m_text[Text].constBegin();
            if (m_text[Text].empty())
                end = m_text[Text].constEnd();
            else {
                const TimeStampedData d = m_text[Text];
                end = d.upperBound(time);
            }
        } else {
            t = Lyric;
            const TimeStampedData d = m_text[Lyric];
            it = d.constBegin();
            end = d.upperBound(time);
        }
        for (; it != end; ++it ) {
            QString s = decodeBytes(it.value());
            appendStringToList(list, s, t);
        }
        return list;
    }

    bool Song::guessTextCodec()
    {
        KEncodingProber prober;
        TimeStampedData::const_iterator it, end;
        if (m_text[Lyric].empty()) {
            it = m_text[Text].constBegin();
            end = m_text[Text].constEnd();
        } else {
            it = m_text[Lyric].constBegin();
            end = m_text[Lyric].constEnd();
        }
        if (it == end)
            return false;
        for (; it != end; ++it )
            prober.feed( it.value() );
        if ( prober.confidence() > 0.6 ) {
            QTextCodec *codec = QTextCodec::codecForName(prober.encoding());
            if (codec == NULL)
                kWarning() << "Unsupported encoding detected:" << prober.encoding();
            else {
                setTextCodec(codec);
                return true;
            }
        }
        return false;
    }

}
