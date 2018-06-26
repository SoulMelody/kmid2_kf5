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

#ifndef EXTERNALSOFTSYNTH_H
#define EXTERNALSOFTSYNTH_H

#include <QObject>
#include <QMap>
#include <QVariant>
#include <QThread>
#include <KProcess>
#include "settings.h"

namespace KMid {
    class MIDIOutput;
}

using namespace KMid;

class ExternalSoftSynth : public QObject {
    Q_OBJECT

public:
    ExternalSoftSynth(Settings* settings);
    virtual ~ExternalSoftSynth();

    void setMidiOutput(MIDIOutput* midiout);
    bool isProgramOK() const;
    QString programVersion() const;
    void saveSettingValues();
    bool settingsChanged();
    bool isOutputReady();
    QStringList getErrorMessages() const;

    virtual void start(bool waiting = false);
    virtual void terminate();
    virtual void check() = 0;

public Q_SLOTS:
    void slotThreadStarted();
    void slotReadStandardError();

Q_SIGNALS:
    void synthErrors(const QString& pgm, const QStringList& messages);
    void synthReady(const QString& pgm, const QStringList& messages);

protected:
    void timerEvent(QTimerEvent *event);
    QStringList readText();
    QString parseVersion( const QString& out );
    QString parseCopyright( const QString& out );
    uint versionNumber(const QString& versionStr);

    bool m_Ok;
    bool m_ready;
    int m_timerId;
    MIDIOutput * m_midiout;
    Settings* m_settings;
    QMap<QString, QVariant> m_oldSettingValues;
    QStringList m_settingsNames;
    QStringList m_warnings;
    KProcess m_process;
    QString  m_prettyName;
    QString  m_version;
    QThread m_thread;
};

class TimiditySoftSynth : public ExternalSoftSynth {
    Q_OBJECT
public:
    TimiditySoftSynth(Settings* settings);
    virtual ~TimiditySoftSynth();

    void start(bool waiting = false);
    void check();

public slots:
    void slotProcessFinished(int, QProcess::ExitStatus);
};

class FluidSoftSynth : public ExternalSoftSynth {
    Q_OBJECT
public:
    FluidSoftSynth(Settings* settings);
    virtual ~FluidSoftSynth();

    void start(bool waiting = false);
    void check();

public slots:
    void slotProcessFinished(int, QProcess::ExitStatus);
};

#endif /* EXTERNALSOFTSYNTH_H */
