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

#ifndef DLSSOFTSYNTH_H
#define DLSSOFTSYNTH_H

#include <QObject>
#include <QStringList>
#include <QMap>
#include <CoreMIDI/MIDIServices.h>
#include <AudioToolbox/AUGraph.h>

namespace KMid {

    class Settings;
    class MacMIDIOutput;

    class DLSSoftSynth : public QObject
    {
        Q_OBJECT
    public:
        explicit DLSSoftSynth( Settings *settings, QObject *parent = 0 );
        virtual ~DLSSoftSynth();

        void start ();
        void terminate ();
        void initialize ( MacMIDIOutput *output );
        bool isInitialized() const;
        void sendEvents ( const MIDIPacketList *packets );
        bool settingsChanged ();
        void saveSettingValues ();
        void registerStatus(const QString& context, const long status);
        QString prettyName() const;

    Q_SIGNALS:
        void synthReady(const QString& pgm, const QStringList& messages);
        void synthErrors(const QString& pgm, const QStringList& messages);

    private:
        AUGraph m_graph;
        AUNode m_synthNode;
        AUNode m_outputNode;
        MIDIEndpointRef m_endpoint;
        Settings* m_settings;
        QMap<QString, QVariant> m_oldSettingValues;
        QStringList m_settingsNames;
        QStringList m_messages;
    };
}

#endif // DLSSOFTSYNTH_H
