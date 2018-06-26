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

#include "winbackend.h"
#include "winmidiobject.h"
#include "winmidioutput.h"

#include <kdemacros.h>
#include <KPluginFactory>
#include <KPluginLoader>

namespace KMid {

    class WinBackend::WinBackendPrivate {
    public:
        WinBackendPrivate():
            m_initialized(false),
            m_backendString("Win"),
            m_object(NULL),
            m_output(NULL)
        {}

        bool m_initialized;
        QString m_backendString;
        WinMIDIObject *m_object;
        WinMIDIOutput *m_output;
    };

    WinBackend::WinBackend(QObject* parent, const QVariantList& args)
        : Backend(parent, args), d(new WinBackendPrivate)
    {
        d->m_object = new WinMIDIObject(this);
        d->m_output = new WinMIDIOutput(this);
        d->m_object->initialize(d->m_output);
        d->m_output->initialize(d->m_object);
        d->m_initialized = true;
    }

    WinBackend::~WinBackend()
    {
        delete d;
    }

    bool WinBackend::initialized()
    {
        return d->m_initialized;
    }

    QString WinBackend::backendName()
    {
        return d->m_backendString;
    }

    MIDIObject* WinBackend::midiObject()
    {
        return d->m_object;
    }

    MIDIOutput* WinBackend::midiOutput()
    {
        return d->m_output;
    }

    K_PLUGIN_FACTORY( WinBackendFactory, registerPlugin<WinBackend>(); )
    K_EXPORT_PLUGIN( WinBackendFactory("kmid_win") )
}

#include "winbackend.moc"
