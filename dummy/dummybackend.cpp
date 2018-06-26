/*
    KMid Dummy Backend
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

#include "dummybackend.h"
#include "dummymidiobject.h"
#include "dummymidioutput.h"

#include <kdemacros.h>
#include <KPluginFactory>
#include <KPluginLoader>

namespace KMid {

    class DummyBackend::BackendPrivate {
    public:
        BackendPrivate():
            m_initialized(false),
            m_backendString("Dummy"),
            m_object(NULL),
            m_output(NULL)
        {}

        bool m_initialized;
        QString m_backendString;
        DummyMIDIObject *m_object;
        DummyMIDIOutput *m_output;
    };

    DummyBackend::DummyBackend(QObject* parent, const QVariantList& args)
        : Backend(parent, args), d(new BackendPrivate)
    {
        d->m_object = new DummyMIDIObject(this);
        d->m_output = new DummyMIDIOutput(this);
        d->m_initialized = true;
    }

    DummyBackend::~DummyBackend()
    {
        delete d;
    }

    bool DummyBackend::initialized()
    {
        return d->m_initialized;
    }

    QString DummyBackend::backendName()
    {
        return d->m_backendString;
    }

    MIDIObject* DummyBackend::midiObject()
    {
        return d->m_object;
    }

    MIDIOutput* DummyBackend::midiOutput()
    {
        return d->m_output;
    }

    bool DummyBackend::hasSoftSynths()
    {
        return false;
    }

    void DummyBackend::setupConfigurationWidget(QWidget* /*widget*/)
    { }

    void DummyBackend::initializeSoftSynths(Settings* /*settings*/)
    { }

    void DummyBackend::terminateSoftSynths()
    { }

    bool DummyBackend::applySoftSynthSettings()
    {
        return false;
    }

    void DummyBackend::updateConfigWidget()
    { }

    void DummyBackend::saveSettings()
    { }

    K_PLUGIN_FACTORY( DummyBackendFactory, registerPlugin<DummyBackend>(); )
    K_EXPORT_PLUGIN( DummyBackendFactory("kmid_dummy") )
}

#include "dummybackend.moc"
