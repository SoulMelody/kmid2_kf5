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

#ifndef MIDIOUTPUT_H
#define MIDIOUTPUT_H

#include "kmidmacros.h"

#include <QObject>

namespace KMid {

    class MidiMapper;

    /**
     * Virtual MIDI OUT port
     */
    class KMIDBACKEND_EXPORT MIDIOutput : public QObject
    {
        Q_OBJECT

    public:
        MIDIOutput(QObject *parent = 0) : QObject(parent) {}
        virtual ~MIDIOutput() {}

        /**
         * Returns the current software volume.
         *
         * A value of 0.0 means muted, 1.0 means unchanged, 2.0 means double
         */
        virtual qreal volume(int channel) const = 0;

        /**
         * Returns the index of the device that is used.
         */
        virtual int outputDevice() const = 0;

        virtual QString outputDeviceName() const = 0;

        virtual QStringList outputDeviceList(bool basicOnly = true) = 0;

        virtual bool isMuted(int channel) const = 0;

        virtual MidiMapper* midiMap() = 0;

        virtual int pitchShift() = 0;

    public Q_SLOTS:

        /**
         * Sets the new current software volume.
         *
         * A value of 0.0 means muted, 1.0 means unchanged, 2.0 means double
         *
         * Every time the volume in the backend changes it should emit volumeChanged(qreal), also
         * inside this function.
         */
        virtual void setVolume(int channel, qreal) = 0;

        /**
         * Requests to change the current output device
         *
         * \returns \c true if the requested device works and is used after this call.
         * \returns \c false if something failed and the device is not used after this call.
         */
        virtual bool setOutputDevice(int) = 0;
        virtual bool setOutputDeviceName(const QString &newOutputDevice) = 0;

        virtual void setMuted(int channel, bool mute) = 0;

        virtual void setLocked(int channel, bool lock) = 0;

        virtual void setMidiMap(MidiMapper *map) = 0;

        virtual void setPitchShift(int amt) = 0;

        virtual void setResetMessage(const QByteArray& msg) = 0;

        /* Realtime MIDI slots */
        virtual void allNotesOff() = 0;
        virtual void resetControllers() = 0;
        virtual void sendResetMessage() = 0;
        virtual void sendNoteOn(int chan, int note, int vel) = 0;
        virtual void sendNoteOff(int chan, int note, int vel) = 0;
        virtual void sendController(int chan, int control, int value) = 0;
        virtual void sendKeyPressure(int chan, int note, int value) = 0;
        virtual void sendProgram(int chan, int program) = 0;
        virtual void sendChannelPressure(int chan, int value) = 0;
        virtual void sendPitchBend(int chan, int value) = 0;
        virtual void sendSysexEvent(const QByteArray& data) = 0;

    Q_SIGNALS:
        /**
         * This signal is emitted whenever the volume has changed. As the
         * volume can change without a call to setVolume (calls over dbus)
         * this is important
         * to keep a widget showing the current volume up to date.
         */
        void volumeChanged(int channel, qreal newVolume);

        /**
         * This signal is emitted when the muted property has changed. As
         * this property can change by IPC (DBus) calls a UI element showing
         * the muted property should listen to this signal.
         */
        void mutedChanged(int channel, bool);

        /**
         * This signal is emitted when the locked property has changed. As
         * this property can change by IPC (DBus) calls a UI element showing
         * the locked property should listen to this signal.
         */
        void lockedChanged(int channel, bool);

        /**
         * This signal is emitted when the (hardware) device for the output
         * has changed.
         *
         * The change can happen either through setOutputDevice or if the
         * global configuration for the used category has changed.
         *
         * \see outputDevice
         */
        void outputDeviceChanged(const QString &newOutputDevice);
    };

}

#endif /* MIDIOUTPUT_H */
