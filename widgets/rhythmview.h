/**************************************************************************

    rhythmview.h  - The RhythmView widget
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

#ifndef RHYTHMVIEW_H
#define RHYTHMVIEW_H

#include <QWidget>
#include <QResizeEvent>
#include <kled.h>

class RhythmView : public QWidget
{
    int last;

    QList<KLed*> lamps;
    
public:
    RhythmView (QWidget *parent = 0);
    ~RhythmView();

    void setRhythm(int numerator);

    void beat(int i); // Sets the beat number

    void beat(); // Just increase the beat

    QSize sizeHint() const;
    QSizePolicy sizePolicy();

private:
    void resizeEvent(QResizeEvent *);
    void clearLamps();

};

#endif /* RHYTHMVIEW_H */
