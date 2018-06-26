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

#ifndef WINBACKEND_H
#define WINBACKEND_H

#include "backend.h"
#include <QObject>

class Settings;

namespace KMid {

    class WinBackend : public Backend
    {
        Q_OBJECT
    public:
        explicit WinBackend(QObject* parent = 0, const QVariantList& args = QVariantList());
        virtual ~WinBackend();

        virtual QString backendName();
        virtual MIDIObject *midiObject();
        virtual MIDIOutput *midiOutput();
        virtual bool initialized();

        /* Soft Synths are not required/supported in Windows */
        virtual bool hasSoftSynths() { return false; }
        virtual void setupConfigurationWidget(QWidget* widget) {}
        virtual void initializeSoftSynths(Settings* settings) {}
        virtual void terminateSoftSynths() {}
        virtual bool applySoftSynthSettings() { return false; }
        virtual void updateConfigWidget() {}
        virtual void saveSettings() {}

    private:
        class WinBackendPrivate;
        WinBackendPrivate *d;
    };

}
#endif // WINBACKEND_H
