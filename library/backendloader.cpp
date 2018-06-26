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

#include "backendloader.h"
#include "backend.h"
#include <KServiceTypeTrader>
#include <KDebug>

namespace KMid {

    BackendLoader::BackendLoader(QObject * parent)
      : QObject(parent)
    { }

    BackendLoader::~BackendLoader()
    { }

    void BackendLoader::loadAllBackends()
    {
        KService::List offers = KServiceTypeTrader::self()->query("KMid/backend");
        foreach (const KService::Ptr &service, offers)
        {
            KPluginFactory *factory = KPluginLoader(service->library()).factory();
            if (factory == NULL) {
                kError() << "KPluginFactory could not load the backend:"
                         << service->library();
                continue;
            }
            Backend *backend = factory->create<Backend>(this, QVariantList());
            if (backend != NULL)
               emit loaded(backend, service->library(), service->name());
        }
    }
}
