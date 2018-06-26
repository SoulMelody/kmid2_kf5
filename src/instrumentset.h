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

#ifndef INSTRUMENTSET_H
#define INSTRUMENTSET_H

#include <QVector>
#include <QStringList>

class InstrumentSet {
public:
    InstrumentSet();
    virtual ~InstrumentSet();

    void reloadNames();

    QString name(const int patch, bool percussion = false) const;

    QStringList names(bool percussion = false) const;

private:
    QVector<QString> m_names;
    QStringList m_numbers;
};

#endif /* INSTRUMENTSET_H */
