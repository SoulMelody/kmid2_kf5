/*
    KMid Backend using the ALSA Sequencer
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

#include "alsabackend.h"
#include "alsamidiobject.h"
#include "alsamidioutput.h"

#include "ui_prefs_progs.h"
#include "externalsoftsynth.h"
#include "settings.h"

#include <kdemacros.h>
#include <KPluginFactory>
#include <KPluginLoader>
#include <KStandardDirs>
#include <KMessageBox>
#include <KLocale>
#include <QWidget>
#include <sched.h>

using namespace drumstick;

namespace KMid {

    class ALSABackend::BackendPrivate {
    public:
        BackendPrivate():
            m_initialized(false),
            m_backendString("ALSA Sequencer"),
            m_object(NULL),
            m_output(NULL),
            m_fluidsynth(NULL),
            m_timidity(NULL),
            m_settings(NULL)
        { }

        ~BackendPrivate()
        {
            delete m_fluidsynth;
            delete m_timidity;
        }

        bool m_initialized;
        QString m_backendString;
        ALSAMIDIObject *m_object;
        ALSAMIDIOutput *m_output;
        FluidSoftSynth* m_fluidsynth;
        TimiditySoftSynth* m_timidity;
        Settings* m_settings;
        Ui::prefs_progs ui_prefs_progs;
    };

    ALSABackend::ALSABackend(QObject* parent, const QVariantList& args)
        : Backend(parent, args), d(new BackendPrivate)
    {
        try {
            d->m_object = new ALSAMIDIObject(this);
            d->m_output = new ALSAMIDIOutput(this);
            d->m_object->initialize(d->m_output);
            d->m_initialized = true;
        } catch (const SequencerError& ex) {
            QString errorstr = i18nc("@info","Fatal error from the ALSA sequencer backend. "
                "This usually happens when the kernel does not have ALSA support, "
                "the device node (/dev/snd/seq) does not exist, "
                "or the kernel module (snd_seq) is not loaded. "
                "Please check your ALSA/MIDI configuration. "
                "Returned error was: %1", ex.qstrError());
            KMessageBox::error(0, errorstr, i18nc("@title:window","ALSA Sequencer Backend Error"));
        } catch (...) {
        	qDebug() << "Fatal error from the ALSA sequencer backend. "
            "This usually happens when the kernel does not have ALSA support, "
            "the device node (/dev/snd/seq) does not exist, "
            "or the kernel module (snd_seq) is not loaded. "
            "Please check your ALSA/MIDI configuration. ";
        }
    }

    ALSABackend::~ALSABackend()
    {
        delete d;
    }

    bool ALSABackend::initialized()
    {
        return d->m_initialized;
    }

    QString ALSABackend::backendName()
    {
        return d->m_backendString;
    }

    MIDIObject* ALSABackend::midiObject()
    {
        return d->m_object;
    }

    MIDIOutput* ALSABackend::midiOutput()
    {
        return d->m_output;
    }

    bool ALSABackend::hasSoftSynths()
    {
        return true;
    }

    void ALSABackend::setupConfigurationWidget(QWidget* widget)
    {
        if (widget != NULL) {
            d->ui_prefs_progs.setupUi(widget);
            /*#if KDE_IS_VERSION(4,3,0)
                QString exepath = KGlobal::dirs()->installPath("exe");
                QString datapath = KGlobal::dirs()->installPath("sound") + "sf2/";
                d->ui_prefs_progs.kcfg_cmd_fluid->setMode(KFile::File|KFile::ExistingOnly|KFile::LocalOnly);
                d->ui_prefs_progs.kcfg_sf2_fluid->setMode(KFile::File|KFile::ExistingOnly|KFile::LocalOnly);
                d->ui_prefs_progs.kcfg_cmd_timidity->setMode(KFile::File|KFile::ExistingOnly|KFile::LocalOnly);
                d->ui_prefs_progs.kcfg_cmd_fluid->setStartDir(exepath);
                d->ui_prefs_progs.kcfg_sf2_fluid->setStartDir(datapath);
                d->ui_prefs_progs.kcfg_cmd_timidity->setStartDir(exepath);
                d->ui_prefs_progs.kcfg_sf2_fluid->setFilter("*.SF2");
             #endif*/
        }
    }

    void ALSABackend::initializeSoftSynths(Settings* settings)
    {
        if (settings == NULL)
            return;
        d->m_settings = settings;
        d->m_fluidsynth = new FluidSoftSynth(settings);
        connect( d->m_fluidsynth,
                 SIGNAL(synthErrors(const QString&, const QStringList&)),
                 SIGNAL(softSynthErrors(const QString&, const QStringList&)) );
        connect( d->m_fluidsynth,
                 SIGNAL(synthReady(const QString&, const QStringList&)),
                 SIGNAL(softSynthStarted(const QString&, const QStringList&)) );
        d->m_fluidsynth->check();
        d->m_fluidsynth->setMidiOutput(d->m_output);
        if (d->m_settings->exec_fluid())
            d->m_fluidsynth->start();
        d->m_timidity = new TimiditySoftSynth(settings);
        connect( d->m_timidity,
                 SIGNAL(synthErrors(const QString&, const QStringList&)),
                 SIGNAL(softSynthErrors(const QString&, const QStringList&)) );
        connect(d->m_timidity,
                SIGNAL(synthReady(const QString&, const QStringList&)),
                SIGNAL(softSynthStarted(const QString&, const QStringList&)) );
        d->m_timidity->check();
        d->m_timidity->setMidiOutput(d->m_output);
        if (d->m_settings->exec_timidity())
            d->m_timidity->start();
    }

    void ALSABackend::terminateSoftSynths()
    {
        d->m_timidity->terminate();
        d->m_fluidsynth->terminate();
    }

    bool ALSABackend::applySoftSynthSettings()
    {
        bool changedFluid(false);
        bool changedTimidity(false);
        changedFluid = d->m_fluidsynth->settingsChanged();
        if (changedFluid) {
            d->m_fluidsynth->terminate();
            d->m_fluidsynth->check();
            if (d->m_settings->exec_fluid())
                d->m_fluidsynth->start(true);
            d->m_fluidsynth->saveSettingValues();
        }
        changedTimidity = d->m_timidity->settingsChanged();
        if (changedTimidity) {
            d->m_timidity->terminate();
            d->m_timidity->check();
            if (d->m_settings->exec_timidity())
                d->m_timidity->start(true);
            d->m_timidity->saveSettingValues();
        }
        return changedTimidity | changedFluid;
    }

    void ALSABackend::updateConfigWidget()
    {
        QString version;
        bool Ok = d->m_fluidsynth->isProgramOK();
        version = d->m_fluidsynth->programVersion();
        d->ui_prefs_progs.label_fluid_available->setPixmap(
            Ok ? QIcon("flag-green").pixmap(24,24) :
                 QIcon("flag-red").pixmap(24,24) );
        d->ui_prefs_progs.label_fluid_available->setText(
            version.isEmpty() ? i18nc("@info","Not found") :
                i18nc("@info","Found version: %1", version) );
        d->ui_prefs_progs.kcfg_exec_fluid->setEnabled(Ok);
        d->ui_prefs_progs.kcfg_audio_fluid->setEnabled(Ok);
        d->ui_prefs_progs.kcfg_args_fluid->setEnabled(Ok);
        d->ui_prefs_progs.kcfg_audiodev_fluid->setEnabled(Ok);
        d->ui_prefs_progs.kcfg_rate_fluid->setEnabled(Ok);

        Ok = d->m_timidity->isProgramOK();
        version = d->m_timidity->programVersion();
        d->ui_prefs_progs.label_timidity_available->setPixmap(
            Ok ? QIcon("flag-green").pixmap(24,24) :
                 QIcon("flag-red").pixmap(24,24) );
        d->ui_prefs_progs.label_timidity_available->setText(
            version.isEmpty() ? i18nc("@info","Not found") :
                i18nc("@info","Found version: %1", version) );
        d->ui_prefs_progs.kcfg_exec_timidity->setEnabled(Ok);
        d->ui_prefs_progs.kcfg_audio_timidity->setEnabled(Ok);
        d->ui_prefs_progs.kcfg_args_timidity->setEnabled(Ok);
        d->ui_prefs_progs.kcfg_audiodev_timidity->setEnabled(Ok);
        d->ui_prefs_progs.kcfg_rate_timidity->setEnabled(Ok);
    }

    void ALSABackend::saveSettings()
    {
        d->m_fluidsynth->saveSettingValues();
        d->m_timidity->saveSettingValues();
    }

    K_PLUGIN_FACTORY( ALSABackendFactory, registerPlugin<ALSABackend>(); )
    K_EXPORT_PLUGIN( ALSABackendFactory("kmid_alsa") )
}

#include "alsabackend.moc"
