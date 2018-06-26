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

#ifndef PIANOLA_H
#define PIANOLA_H

#include "midimapper.h"
#include <KMainWindow>

class QSignalMapper;
class QFrame;
class KToggleAction;
class PianoKeybd;
class QLabel;

class Pianola : public KMainWindow {
    Q_OBJECT

public:
    Pianola( QWidget* parent = 0 );
    virtual ~Pianola();
    void enableChannel(int channel, bool enable);
    void setNoteRange(int lowerNote, int upperNote);

signals:
    void closed();
    void noteOn(int channel, int note, int vel);
    void noteOff(int channel, int note, int vel);

public slots:
    void playNoteOn(int note);
    void playNoteOff(int note);
    void slotNoteOn(int channel, int note, int vel);
    void slotNoteOff(int channel, int note, int vel);
    void slotShowChannel(int chan);
    void slotShowAllChannels();
    void slotHideAllChannels();
    void allNotesOff();
    void slotLabel(int channel, const QString& text);

protected:
    bool queryClose();
    void showEvent( QShowEvent * event );

private:
    QVector<PianoKeybd*> m_piano;
    QVector<QFrame*> m_frame;
    QVector<KToggleAction*> m_action;
    QVector<QLabel*> m_label;
    QSignalMapper* m_mapper;
};

#endif /* PIANOLA_H */
