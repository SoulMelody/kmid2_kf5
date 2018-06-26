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

#include "externalsoftsynth.h"
#include "settings.h"
#include "midioutput.h"

#include <KConfigSkeleton>
#include <KMessageBox>
#include <KStandardDirs>
#include <KLocale>
#include <QTextStream>
#include <QFileInfo>
#include <KGlobal>
#include <KUrl>

const int STARTUP_TIMEOUT(1000);

ExternalSoftSynth::ExternalSoftSynth(Settings* settings):
    QObject(0),
    m_Ok(false),
    m_ready(false),
    m_timerId(0),
    m_midiout(0),
    m_settings(settings)
{
    qRegisterMetaType<QProcess::ExitStatus>("QProcess::ExitStatus");
    m_prettyName.clear();
    m_version.clear();
    moveToThread(&m_thread);
    connect(&m_thread, SIGNAL(started()), SLOT(slotThreadStarted()));
}

ExternalSoftSynth::~ExternalSoftSynth()
{
    terminate();
}

bool ExternalSoftSynth::isProgramOK() const
{
    return m_Ok;
}

QString ExternalSoftSynth::programVersion() const
{
    return m_version;
}

QStringList ExternalSoftSynth::getErrorMessages() const
{
    return m_warnings;
}

bool ExternalSoftSynth::settingsChanged()
{
    foreach( const QString& propName, m_settingsNames ) {
        KConfigSkeletonItem* itm = m_settings->findItem(propName);
        if (itm != NULL && !itm->isEqual(m_oldSettingValues[propName])) {
            return true;
        }
    }
    return false;
}

void ExternalSoftSynth::saveSettingValues()
{
    foreach( const QString& propName, m_settingsNames ) {
        KConfigSkeletonItem* itm = m_settings->findItem(propName);
        if (itm != NULL)
            m_oldSettingValues[propName] = itm->property();
    }
}

uint ExternalSoftSynth::versionNumber(const QString& versionStr)
{
    bool ok;
    uint result = 0;
    QStringList lst = versionStr.split('.');
    foreach (const QString& n, lst) {
        result <<= 8;
        result += n.toInt(&ok);
    }
    return result;
}

QString ExternalSoftSynth::parseVersion(const QString& out )
{
    int pos = out.indexOf( m_prettyName );
    if( pos < 0 )
        return QString();

    int sPos = out.indexOf( QRegExp("\\d"), pos );
    if( sPos < 0 )
        return QString();

    int endPos = out.indexOf( QRegExp("[\\s,]"), sPos + 1 );
    if( endPos < 0 )
        return QString();

    return out.mid( sPos, endPos - sPos );
}

QString ExternalSoftSynth::parseCopyright( const QString& out )
{
    int pos = out.indexOf( "(C)", 0 );
    if ( pos < 0 )
        return QString();
    pos += 4;
    int endPos = out.indexOf( '\n', pos );
    return out.mid( pos, endPos-pos );
}

QStringList ExternalSoftSynth::readText()
{
    QTextStream stream(&m_process);
    QStringList txt;
    QString line;
    do {
        line = stream.readLine();
        if (!line.isEmpty())
            txt += line;
    } while (!line.isEmpty());
    return txt;
}

void ExternalSoftSynth::terminate()
{
    if (m_process.state() == KProcess::Running) {
        m_process.disconnect();
        m_process.kill();
        m_process.waitForFinished(STARTUP_TIMEOUT);
    }
}

void ExternalSoftSynth::timerEvent(QTimerEvent* event)
{
    Q_UNUSED(event);
    if ( m_process.state() == KProcess::Starting )
        return;
    m_ready = isOutputReady();
    if (m_ready || (m_process.state() != KProcess::Running ) ) {
        if (m_timerId != 0) {
            killTimer(m_timerId);
            m_timerId = 0;
        }
        m_thread.quit();
        if ((m_process.state() == KProcess::Running))
            emit synthReady(m_prettyName, m_warnings);
    }
}

void ExternalSoftSynth::setMidiOutput(MIDIOutput* midiout)
{
    m_midiout = midiout;
}

bool ExternalSoftSynth::isOutputReady()
{
    if (m_midiout != NULL) {
        QStringList lst = m_midiout->outputDeviceList();
        foreach(const QString& s, lst)
            if (s.contains(m_settings->output_connection(), Qt::CaseInsensitive))
                return true;
    }
    return false;
}

void ExternalSoftSynth::start(bool waiting)
{
    m_ready = false;
    m_warnings.clear();
    m_process.start();
    m_thread.start();
    if ((m_process.state() == KProcess::Running) && waiting)
        m_thread.wait();
}

void ExternalSoftSynth::slotThreadStarted()
{
    m_timerId = startTimer(STARTUP_TIMEOUT);
}

void ExternalSoftSynth::slotReadStandardError()
{
    m_process.setReadChannel(QProcess::StandardError);
    m_warnings += readText();
}

/*
 * TimiditySoftSynth
 * =================
 */

TimiditySoftSynth::TimiditySoftSynth(Settings* settings):
    ExternalSoftSynth(settings)
{
    m_prettyName = "TiMidity++";
    m_settingsNames << "exec_timidity";
    m_settingsNames << "cmd_timidity";
    m_settingsNames << "audio_timidity";
    m_settingsNames << "audiodev_timidity";
    m_settingsNames << "rate_timidity";
    m_settingsNames << "args_timidity";
}

TimiditySoftSynth::~TimiditySoftSynth()
{ }

void TimiditySoftSynth::check()
{
    KProcess timidity;
    KUrl u = m_settings->cmd_timidity();
    QString cmd = KGlobal::dirs()->findExe(u.toLocalFile());
    m_Ok = false;
    m_version.clear();
    if (cmd.isEmpty())
        return;
    timidity.setOutputChannelMode( KProcess::MergedChannels );
    timidity << cmd << "--version";
    if( timidity.execute( STARTUP_TIMEOUT ) >= 0 ) {
        QString s = QString::fromLocal8Bit( timidity.readAll() );
        m_version = parseVersion(s);
        QString copyright = parseCopyright(s);
        m_Ok = !m_version.isEmpty() && !copyright.isEmpty();
    }
}

void TimiditySoftSynth::start(bool waiting)
{
    QStringList args;
    if ( m_process.state() == KProcess::NotRunning && m_Ok ) {
        args += "-iA";
        switch (m_settings->audio_timidity()) {
        case Settings::timidity_alsa:
            args += "-Os";
            if ( !m_settings->audiodev_timidity().isEmpty() )
                args += ("-o" + m_settings->audiodev_timidity());
            break;
        case Settings::timidity_pulseaudio:
            args += "-OO"; // libao
            args += "-opulse"; // use libao's "pulse" device
            break;
        case Settings::timidity_oss:
            args += "-Od";
            if ( !m_settings->audiodev_timidity().isEmpty() )
                args += ("-o" + m_settings->audiodev_timidity());
            break;
        case Settings::timidity_jack:
            args += "-Oj";
            break;
        }
        if (!m_settings->rate_timidity().isEmpty())
            args += ("-s" + m_settings->rate_timidity());
        if (!m_settings->args_timidity().isEmpty())
            args += m_settings->args_timidity().split(QRegExp("\\s+"));

        connect( &m_process, SIGNAL(finished(int, QProcess::ExitStatus)),
                 SLOT(slotProcessFinished(int, QProcess::ExitStatus)) );
        connect( &m_process, SIGNAL(readyReadStandardError()),
                 SLOT(slotReadStandardError()) );

        m_settings->setOutput_connection(QLatin1String("TiMidity:0"));
        m_process.setOutputChannelMode( KProcess::OnlyStderrChannel );
        m_process.setProgram(m_settings->cmd_timidity().toLocalFile(), args);
        ExternalSoftSynth::start(waiting);
    }
}

void TimiditySoftSynth::slotProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode);
    Q_UNUSED(exitStatus);
    if (m_timerId != 0) {
        killTimer(m_timerId);
        m_timerId = 0;
    }
    m_thread.quit();
    slotReadStandardError();
    m_settings->setExec_timidity(false);
    emit synthErrors(m_prettyName, m_warnings);
}

/*
 * FluidSoftSynth
 * ==============
 */

FluidSoftSynth::FluidSoftSynth(Settings* settings):
    ExternalSoftSynth(settings)
{
    m_prettyName = "FluidSynth";
    m_settingsNames << "exec_fluid";
    m_settingsNames << "cmd_fluid";
    m_settingsNames << "audio_fluid";
    m_settingsNames << "audiodev_fluid";
    m_settingsNames << "rate_fluid";
    m_settingsNames << "sf2_fluid";
    m_settingsNames << "args_fluid";
}

FluidSoftSynth::~FluidSoftSynth()
{ }

void FluidSoftSynth::check()
{
    KProcess fluidsynth;
    KUrl u = m_settings->cmd_fluid();
    QString cmd = KGlobal::dirs()->findExe(u.toLocalFile());
    m_Ok = false;
    m_version.clear();
    if (cmd.isEmpty())
        return;
    fluidsynth.setOutputChannelMode( KProcess::MergedChannels );
    fluidsynth << cmd << "--version";
    if( fluidsynth.execute( STARTUP_TIMEOUT ) >= 0 ) {
        QString s = QString::fromLocal8Bit( fluidsynth.readAll() );
        m_version = parseVersion(s);
        m_Ok = !m_version.isEmpty() && !m_settings->sf2_fluid().isEmpty();
        m_Ok &= (versionNumber(m_version) >= 0x010009U);
        QFileInfo info(m_settings->sf2_fluid().toLocalFile());
        m_Ok &= info.exists();
    }
}

void FluidSoftSynth::start(bool waiting)
{
    QStringList args;
    if ( m_process.state() == KProcess::NotRunning && m_Ok ) {
        args += "--disable-lash";
        args += "--portname=FluidSynth";
        args += "--midi-driver=alsa_seq";
        switch (m_settings->audio_fluid()) {
        case Settings::fluid_alsa:
            args += "--audio-driver=alsa";
            if (!m_settings->audiodev_fluid().isEmpty())
                args += ("-oaudio.alsa.device=" + m_settings->audiodev_fluid());
            break;
        case Settings::fluid_pulseaudio:
            args += "--audio-driver=pulseaudio";
            if (!m_settings->audiodev_fluid().isEmpty())
                args += ("-oaudio.pulseaudio.device=" + m_settings->audiodev_fluid());
            break;
        case Settings::fluid_oss:
            args += "--audio-driver=oss";
            if (!m_settings->audiodev_fluid().isEmpty())
                args += ("-oaudio.oss.device=" + m_settings->audiodev_fluid());
            break;
        case Settings::fluid_jack:
            args += "--audio-driver=jack";
            args += "--connect-jack-outputs";
            break;
        }
        if (!m_settings->rate_fluid().isEmpty())
            args += ("--sample-rate=" + m_settings->rate_fluid());
        if (!m_settings->args_fluid().isEmpty()) {
            QStringList args_fluid = m_settings->args_fluid().split(QRegExp("\\s+"));
            foreach( const QString& arg, args_fluid)
                if (arg != "-i" && arg != "--no-shell")
                    args += arg;
        }
        args += m_settings->sf2_fluid().toLocalFile();

        connect( &m_process, SIGNAL(finished(int, QProcess::ExitStatus)),
                 SLOT(slotProcessFinished(int, QProcess::ExitStatus)) );
        connect( &m_process, SIGNAL(readyReadStandardError()),
                 SLOT(slotReadStandardError()) );

        m_settings->setOutput_connection(QLatin1String("FluidSynth:0"));
        m_process.setOutputChannelMode( KProcess::OnlyStderrChannel );
        m_process.setProgram(m_settings->cmd_fluid().toLocalFile(), args);
        ExternalSoftSynth::start(waiting);
    }
}

void FluidSoftSynth::slotProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode);
    Q_UNUSED(exitStatus);
    if (m_timerId != 0) {
        killTimer(m_timerId);
        m_timerId = 0;
    }
    m_thread.quit();
    slotReadStandardError();
    m_settings->setExec_fluid(false);
    emit synthErrors(m_prettyName, m_warnings);
}
