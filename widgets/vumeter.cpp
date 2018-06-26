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

#include "vumeter.h"

#include <QtGui/QImage>
#include <QtGui/QPainter>
#include <QtGui/QBrush>
#include <QtGui/QPen>
#include <QtCore/QDebug>

Vumeter::Vumeter(QWidget *parent) :
    QWidget(parent),
    m_img(0),
    m_max(1.0),
    m_value(0.0)
{
    setMinimumWidth( 100 );
    setFixedHeight( 20 );
    initImage();
}

Vumeter::~Vumeter()
{
    delete m_img;
}

void Vumeter::initImage()
{
    delete m_img;
    m_img = new QImage(width(), height(), QImage::Format_ARGB32);
    m_img->fill(0);
}

void Vumeter::paintEvent(QPaintEvent *)
{
    static const QColor colBg = QColor(0, 50, 0);
    QPainter painter(this);
    painter.setBrush( isEnabled() ? colBg : palette().dark() );
    painter.drawRect( QRectF(0, 0, width(), height()) );
    if (!isEnabled())
        return;
    painter.drawImage( 0, 0,
        m_img->copy(0, 0, width() * m_value / m_max, height()) );
}

void Vumeter::resizeEvent(QResizeEvent *)
{
    static const QColor colHi(Qt::red);
    static const QColor colLo(Qt::green);
    static const QColor colBg(0, 50, 0);
    static const qreal  margin(1.0);

    initImage();
    QPainter painter(m_img);
    painter.setRenderHint(QPainter::Antialiasing);
    // background
    painter.setBrush(colBg);
    painter.drawRect(QRectF(0, 0, width(), height()));
    // value
    QLinearGradient grad(0, 0, width(), height());
    grad.setColorAt(0, colLo);
    grad.setColorAt(1, colHi);
    painter.setBrush(grad);
    painter.drawRect( QRectF( margin, margin,
            width() - 2.0 * margin, height() - 2.0 * margin));
    // ticks
    painter.setPen(colBg);
    painter.setBrush(colBg);
    qreal c = width() / (4.0 * margin);
    int m = c / m_max;
    for ( int i = 0; i < m; ++i )
        painter.drawRect(QRectF(width()*i/c, 0, margin, height()));
}

void Vumeter::setValue(qreal value)
{
    if (isEnabled()) {
        if (value > m_max)
            m_value = m_max;
        else
            m_value = value;
        update();
    }
}

qreal Vumeter::value() const
{
    return m_value;
}

qreal Vumeter::decay(qreal pct)
{
    if (isEnabled()) {
        m_value -= (m_max * pct / 100);
        if (m_value < 0) m_value = 0.0;
        update();
    }
    return m_value;
}

void Vumeter::setMaxValue(qreal maxValue)
{
    m_max = maxValue;
    update();
}

qreal Vumeter::maxValue() const
{
    return m_max;
}

QSize Vumeter::minimumSizeHint() const
{
    return QSize(100, 20);
}

QSize Vumeter::sizeHint() const
{
    return QSize(100, 20);
}
