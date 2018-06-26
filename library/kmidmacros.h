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

#ifndef KMIDMACROS_H
#define KMIDMACROS_H

#include <kdemacros.h>

#ifdef MAKE_KMIDBACKEND_LIB
#define KMIDBACKEND_EXPORT KDE_EXPORT
#else /*MAKE_KMIDBACKEND_LIB*/
#define KMIDBACKEND_EXPORT KDE_IMPORT
#endif /*MAKE_KMIDBACKEND_LIB*/

#endif /* KMIDMACROS_H */
