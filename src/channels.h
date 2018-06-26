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

#ifndef CHANNELS_H
#define CHANNELS_H

#include "midimapper.h"
#include "instrumentset.h"
#include <KMainWindow>

class QSignalMapper;
class QToolButton;
class KComboBox;
class Vumeter;
class KLineEdit;

class Channels : public KMainWindow {
    Q_OBJECT

public:
    Channels( QWidget* parent = 0 );
    virtual ~Channels();
    void enableChannel(int channel, bool enable);
    qreal volumeFactor();
    void setVolumeFactor(qreal factor);

    QString channelName(int channel) const;
    bool isChannelMuted(int channel) const;
    bool isChannelSoloed(int channel) const;
    bool isChannelLocked(int channel) const;
    int channelPatch(int channel) const;

    void setChannelName(int channel, const QString& name);
    void setMuteChannel(int channel, bool mute);
    void setSoloChannel(int channel, bool solo);
    void setLockChannel(int channel, bool lock);
    void setPatchChannel(int channel, int patch);

signals:
    void closed();
    void patch(int channel, int value);
    void mute(int channel, bool muted);
    void volume(int channel, qreal factor);
    void lock(int channel, bool locked);
    void name(int channel, const QString& text);

public slots:
    void slotPatch(int channel, int value);
    void slotNoteOn(int channel, int note, int vel);
    void slotNoteOff(int channel, int note, int vel);
    void slotEnableAllChannels();
    void slotDisableAllChannels();
    void slotMuteChannel(int channel);
    void slotSoloChannel(int channel);
    void slotPatchChanged(int channel);
    void slotLockChannel(int channel);
    void slotNameChannel(int channel);
    void allNotesOff();

protected:
    bool queryClose();
    void timerEvent( QTimerEvent *event );

private:
    int m_timerId;
    qreal m_volumeFactor;
    InstrumentSet m_instSet;
    int m_voices[MIDI_CHANNELS];
    qreal m_level[MIDI_CHANNELS];
    qreal m_factor[MIDI_CHANNELS];
    QToolButton* m_mute[MIDI_CHANNELS];
    QToolButton* m_solo[MIDI_CHANNELS];
    QToolButton* m_lock[MIDI_CHANNELS];
    Vumeter* m_vumeter[MIDI_CHANNELS];
    KComboBox* m_patch[MIDI_CHANNELS];
    KLineEdit* m_name[MIDI_CHANNELS];
    QSignalMapper* m_muteMapper;
    QSignalMapper* m_soloMapper;
    QSignalMapper* m_patchMapper;
    QSignalMapper* m_lockMapper;
    QSignalMapper* m_nameMapper;
};

#endif /* CHANNELS_H */
