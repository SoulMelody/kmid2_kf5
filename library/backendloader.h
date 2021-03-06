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

#ifndef BACKENDLOADER_H
#define BACKENDLOADER_H

#include "kmidmacros.h"

#include <QObject>

namespace KMid {

    class Backend;

    class KMIDBACKEND_EXPORT BackendLoader : public QObject
    {
        Q_OBJECT
        public:
            BackendLoader(QObject *parent = 0);
            virtual ~BackendLoader();

            void loadAllBackends();

        signals:
            void loaded( Backend *backend,
                         const QString& library,
                         const QString& name );
    };

}

#endif /* BACKENDLOADER_H */
