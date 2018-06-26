/*
    KMid2 MIDI/Karaoke Player
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

#include "timelabel.h"
#include <QTime>
#include <QPainter>
#include <QDebug>

TimeLabel::TimeLabel(QWidget *parent)
    : QLabel(parent),
    m_time(0),
    m_markers(0)
{ }

void TimeLabel::setTotalTime(qint64 time)
{
    if (m_time != time) {
        m_time = time;
        update();
    }
}

qint64 TimeLabel::totalTime() const
{
    return m_time;
}

void TimeLabel::setMarkers(int num)
{
    if (m_markers != num) {
        m_markers = num;
        update();
    }
}

int TimeLabel::markers() const
{
    return m_markers;
}

void TimeLabel::paintEvent(QPaintEvent* event)
{
    QPainter p(this);
    QFontMetrics fm(font());
    int w = width() / m_markers;
    int free = w;
    for (int i=1; i<=m_markers; ++i) {
        QTime t;
        t = t.addMSecs(m_time * i / m_markers);
        QString s = t.toString("m:ss");
        int tw = fm.width(s);
        if (tw < free) {
            QRectF r = rect();
            r.setWidth(free);
            r.moveRight(w * i);
            p.drawText(r, Qt::AlignRight | Qt::AlignTop, s);
            free = w;
        } else
            free += w;
    }
    Q_UNUSED(event);
}
