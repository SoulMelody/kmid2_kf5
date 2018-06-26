/**************************************************************************

    rhythmview.cpp  - The RhythmView widget
    Copyright (C) 1998  Antonio Larrosa Jimenez <larrosa@kde.org>
    Copyright (C) 2009-2010  Pedro Lopez-Cabanillas <plcl@users.sf.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

***************************************************************************/

#include "rhythmview.h"

RhythmView::RhythmView (QWidget *parent)
    : QWidget(parent),
    last(0)
{
    setRhythm( 4 );
}

RhythmView::~RhythmView()
{
    clearLamps();
}

void RhythmView::clearLamps()
{
    int i;
    if (!lamps.isEmpty()) {
        for (i=0; i<lamps.size(); ++i)
            delete lamps[i];

        lamps.clear();
    }
}

void RhythmView::setRhythm( int numerator )
{
    if (lamps.size() != numerator) {
        int i;
        int w = width() / numerator;
        int x = 0;
        clearLamps();
        for ( i=0; i<numerator; ++i ) {
            KLed *led = new KLed( Qt::yellow, KLed::Off, KLed::Sunken,
                                  KLed::Rectangular, this );
            led->setGeometry(x+2,0,w-4,height());
            led->show();
            lamps.insert(i, led);
            x += w;
        }
        lamps[0]->on();
        last = 0;
    }
}

void RhythmView::beat(int j)
{
    int i;
    if (j>lamps.size()) setRhythm(j); // This is a preventive case

    for ( i=0; i<lamps.size(); ++i )
        lamps[i]->off();

    last = j-1;
    lamps[last]->on();
}

void RhythmView::beat()
{
    int i;
    for ( i=0; i<lamps.size(); ++i )
        lamps[i]->off();

    ++last;
    if (last >= lamps.size())
        last = 0;

    lamps[last]->on();
}

void RhythmView::resizeEvent(QResizeEvent *)
{
    int i;
    int w = width()/lamps.size();
    int x = 0;

    for ( i=0; i<lamps.size(); i++ ) {
        lamps[i]->setGeometry(x+2,0,w-4,height());
        x += w;
    }
}

QSize RhythmView::sizeHint() const
{
    return QSize(10,10);
}

QSizePolicy RhythmView::sizePolicy()
{
    return QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);
}
