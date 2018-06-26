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

#ifndef BACKEND_H
#define BACKEND_H

#include "kmidmacros.h"
#include "midiobject.h"
#include "midioutput.h"
#include "settings.h"

#include <QObject>
#include <QString>
#include <QVariantList>

namespace KMid {

    class KMIDBACKEND_EXPORT Backend : public QObject
    {
        Q_OBJECT

        public:
            explicit Backend(QObject* parent = 0, const QVariantList& args = QVariantList())
                : QObject(parent) { Q_UNUSED(args) }
            virtual ~Backend() {}

            virtual QString backendName() = 0;

            virtual MIDIObject *midiObject() = 0;

            virtual MIDIOutput *midiOutput() = 0;

            virtual bool initialized() = 0;

            virtual bool hasSoftSynths() = 0;

            virtual void setupConfigurationWidget(QWidget* widget) = 0;

            virtual void initializeSoftSynths(Settings* settings) = 0;

            virtual void terminateSoftSynths() = 0;

            virtual bool applySoftSynthSettings() = 0;

            virtual void updateConfigWidget() = 0;

            virtual void saveSettings() = 0;

        Q_SIGNALS:

            void softSynthStarted(const QString& pgm, const QStringList& messages);
            void softSynthErrors(const QString& pgm, const QStringList& messages);

    };
}

#endif /* BACKEND_H */
