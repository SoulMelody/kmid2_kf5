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

#ifndef DUMMYMIDIOUTPUT_H
#define DUMMYMIDIOUTPUT_H

#include "midioutput.h"
#include <QObject>

namespace KMid {
    class DummyMIDIOutput : public MIDIOutput
    {
        Q_OBJECT

    public:
        explicit DummyMIDIOutput(QObject *parent = 0);
        virtual ~DummyMIDIOutput();

        virtual qreal volume(int channel) const;
        virtual int outputDevice() const;
        virtual QString outputDeviceName() const;
        virtual QStringList outputDeviceList(bool basicOnly = true);
        virtual bool isMuted(int channel) const;
        virtual MidiMapper* midiMap();
        virtual int pitchShift();

    public Q_SLOTS:
        void setVolume(int channel, qreal);
        bool setOutputDevice(int);
        bool setOutputDeviceName(const QString &newOutputDevice);
        void setMuted(int channel, bool mute);
        void setLocked(int channel, bool lock);
        void setMidiMap(MidiMapper *map);
        void setPitchShift(int amt);
        void setResetMessage(const QByteArray& msg);
        void reloadDeviceList();

        /* Realtime MIDI slots */
        void allNotesOff();
        void resetControllers();
        void sendResetMessage();
        void sendNoteOn(int chan, int note, int vel);
        void sendNoteOff(int chan, int note, int vel);
        void sendController(int chan, int control, int value);
        void sendKeyPressure(int chan, int note, int value);
        void sendProgram(int chan, int program);
        void sendChannelPressure(int chan, int value);
        void sendPitchBend(int chan, int value);
        void sendSysexEvent(const QByteArray& data);

    private:
        class DummyMIDIOutputPrivate;
        DummyMIDIOutputPrivate *d;
    };
}

#endif // DUMMYMIDIOUTPUT_H
