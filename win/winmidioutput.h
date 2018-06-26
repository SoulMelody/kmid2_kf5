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

#ifndef WINMIDIOUTPUT_H
#define WINMIDIOUTPUT_H

#include "midioutput.h"
#include <QObject>
#include <windows.h>
#include <mmsystem.h>

#define MIDI_STATUS_NOTEOFF         0x80
#define MIDI_STATUS_NOTEON          0x90
#define MIDI_STATUS_KEYPRESURE      0xa0
#define MIDI_STATUS_CONTROLCHANGE   0xb0
#define MIDI_STATUS_PROGRAMCHANGE   0xc0
#define MIDI_STATUS_CHANNELPRESSURE 0xd0
#define MIDI_STATUS_PITCHBEND       0xe0
#define MIDI_STATUS_SYSEX           0xf0
#define MIDI_STATUS_ENDSYSEX        0xf7
#define MIDI_STATUS_REALTIME        0xf8
#define MIDI_STATUS_MASK            0xf0
#define MIDI_CHANNEL_MASK           0x0f

#define MIDI_LSB(x) (x % 0x80)
#define MIDI_MSB(x) (x / 0x80)

#define KMID_STREAM_BUFMAX 2000

namespace KMid {

    class WinMIDIObject;
    class Song;
    class SongEvent;

    union WinMIDIPacket {
        WinMIDIPacket() : dwPacket(0) {};
        DWORD dwPacket;
        quint8 data[sizeof(DWORD)];
    };

    class WinMIDIOutput : public MIDIOutput
    {
        Q_OBJECT

    public:
        explicit WinMIDIOutput(QObject *parent = 0);
        virtual ~WinMIDIOutput();

        void initialize(WinMIDIObject *output);
        virtual qreal volume(int channel) const;
        virtual int outputDevice() const;
        virtual QString outputDeviceName() const;
        virtual QStringList outputDeviceList(bool basicOnly = true);
        virtual bool isMuted(int channel) const;
        virtual MidiMapper* midiMap();
        virtual int pitchShift();
        void sendShortMessage(WinMIDIPacket &msg, bool discardable = true);

        /* Stream IO Methods */
        bool openStream(unsigned int dev);
        void closeStream();
        void songOutput();
        void doneHeader( LPMIDIHDR lpMidiHdr );
        void streamPosition( LPMIDIHDR lpMidiHdr );
        void setTimeDivision(int division);
        void outEvent(int &lastTick, bool &first, SongEvent *ev);

        /* Delegated methods from WinMIDIObject */
        void setTimeSkew(qreal skew);
        void setTickInterval(qint32 interval);
        void setInitialTempo(int initialTempo);
        bool play();
        bool pause();
        bool stop();
        bool seek(qint64 time);
        qint32 tickInterval() const;
        qint64 currentTime() const;
        qreal currentTempo();
        qreal timeSkew();
        void resetPosition();
        void setSong(Song* s);

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
        void sendInitialProgram(int channel, int value);

    Q_SIGNALS:
        void finished();

    private:
        class WinMIDIOutputPrivate;
        WinMIDIOutputPrivate *d;
    };
}

#endif // WINMIDIOUTPUT_H
