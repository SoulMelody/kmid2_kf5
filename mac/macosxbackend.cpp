/*
    KMid Mac OSX Backend
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

#include "macosxbackend.h"
#include "macmidiobject.h"
#include "macmidioutput.h"
#include "dlssoftsynth.h"
#include "settings.h"
#include "ui_prefs_synth.h"

#include <kdemacros.h>
#include <KPluginFactory>
#include <KPluginLoader>

namespace KMid {

    class MacOSXBackend::BackendPrivate {
    public:
        BackendPrivate():
            m_initialized(false),
            m_backendString("MacOSX"),
            m_object(NULL),
            m_output(NULL),
            m_synth(NULL)
        { }

        bool m_initialized;
        QString m_backendString;
        MacMIDIObject *m_object;
        MacMIDIOutput *m_output;
        DLSSoftSynth *m_synth;
        Settings *m_settings;
        Ui::prefs_synth ui_prefs_synth;
    };

    MacOSXBackend::MacOSXBackend(QObject* parent, const QVariantList& args)
        : Backend(parent, args), d(new BackendPrivate)
    {
        d->m_object = new MacMIDIObject(this);
        d->m_output = new MacMIDIOutput(this);
        d->m_object->initialize(d->m_output);
        d->m_initialized = true;
    }

    MacOSXBackend::~MacOSXBackend()
    {
        delete d;
    }

    bool MacOSXBackend::initialized()
    {
        return d->m_initialized;
    }

    QString MacOSXBackend::backendName()
    {
        return d->m_backendString;
    }

    MIDIObject* MacOSXBackend::midiObject()
    {
        return d->m_object;
    }

    MIDIOutput* MacOSXBackend::midiOutput()
    {
        return d->m_output;
    }

    bool MacOSXBackend::hasSoftSynths()
    {
        return true;
    }

    void MacOSXBackend::setupConfigurationWidget(QWidget* widget)
    {
        if (widget != NULL)
            d->ui_prefs_synth.setupUi(widget);
    }

    void MacOSXBackend::initializeSoftSynths(Settings* settings)
    {
        d->m_settings = settings;
        d->m_synth = new DLSSoftSynth(settings, this);
        connect( d->m_synth,
                 SIGNAL(synthErrors(const QString&, const QStringList&)),
                 SIGNAL(softSynthErrors(const QString&, const QStringList&)) );
        connect( d->m_synth,
                 SIGNAL(synthReady(const QString&, const QStringList&)),
                 SIGNAL(softSynthStarted(const QString&, const QStringList&)) );
        if (d->m_settings->exec_dls()) {
            d->m_synth->initialize(d->m_output);
            d->m_synth->start();
            d->m_synth->saveSettingValues();
            d->m_output->reloadDeviceList();
        }
    }

    void MacOSXBackend::terminateSoftSynths()
    {
        if (d->m_synth != NULL)
            d->m_synth->terminate();
    }

    bool MacOSXBackend::applySoftSynthSettings()
    {
        if (d->m_synth == NULL)
            return false;
        bool changed = d->m_synth->settingsChanged();
        if (changed) {
            d->m_synth->terminate();
            if (d->m_settings->exec_dls()) {
                if (!d->m_synth->isInitialized())
                    d->m_synth->initialize(d->m_output);
                d->m_synth->start();
                d->m_output->reloadDeviceList();
            }
            d->m_synth->saveSettingValues();
        }
        return changed;
    }

    void MacOSXBackend::updateConfigWidget()
    { }

    void MacOSXBackend::saveSettings()
    {
        if (d->m_synth != NULL)
            d->m_synth->saveSettingValues();
    }

    K_PLUGIN_FACTORY( MacOSXBackendFactory, registerPlugin<MacOSXBackend>(); )
    K_EXPORT_PLUGIN( MacOSXBackendFactory("kmid_mac") )
}

#include "macosxbackend.moc"
