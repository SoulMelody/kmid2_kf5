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

#include "pianola.h"
#include "midimapper.h"
#include "pianokeybd.h"

#include <QSignalMapper>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QFrame>
#include <QLabel>
#include <QFont>
#include <KGlobalSettings>
#include <KLocale>
#include <KMenuBar>
#include <KF5/KWidgetsAddons/KToggleAction>
#include "KF5/KDELibs4Support/kaction.h"

Pianola::Pianola( QWidget* parent ) : KMainWindow(parent)
{
    setObjectName("PlayerPianoWindow");
    setAttribute(Qt::WA_DeleteOnClose, false);
    setCaption(i18nc("@title:window","Player Piano"));
    m_piano.resize(MIDI_CHANNELS);
    m_action.resize(MIDI_CHANNELS);
    m_frame.resize(MIDI_CHANNELS);
    m_label.resize(MIDI_CHANNELS);
    m_mapper = new QSignalMapper(this);
    QMenu* chmenu = menuBar()->addMenu(i18nc("@title:menu","MIDI Channels"));
    QAction *a = new QAction(this);
    a->setText(i18nc("@action:inmenu","Show all channels"));
    connect(a, SIGNAL(triggered()), SLOT(slotShowAllChannels()));
    chmenu->addAction(a);
    a = new QAction(this);
    a->setText(i18nc("@action:inmenu","Hide all channels"));
    connect(a, SIGNAL(triggered()), SLOT(slotHideAllChannels()));
    chmenu->addAction(a);
    QVBoxLayout *vlayout = new QVBoxLayout;
    vlayout->setSpacing(0);
    vlayout->setContentsMargins(0,0,0,0);
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    centralWidget->setLayout(vlayout);
    for (int i = 0; i < MIDI_CHANNELS; ++i ) {
        m_frame[i] = new QFrame(this);
        QGridLayout* glayout = new QGridLayout;
        glayout->setSpacing(0);
        glayout->setContentsMargins(0,0,0,0);
        m_frame[i]->setLayout(glayout);
        QLabel* lbl = new QLabel(this);
        lbl->setNum(i+1);
        lbl->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        lbl->setMinimumWidth(25);
        glayout->addWidget(lbl,0,0,2,1);
        m_label[i] = new QLabel(this);
        m_label[i]->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
        glayout->addWidget(m_label[i],0,1);
        m_piano[i] = new PianoKeybd(this);
        connect(m_piano[i], SIGNAL(noteOn(int)), SLOT(playNoteOn(int)));
        connect(m_piano[i], SIGNAL(noteOff(int)), SLOT(playNoteOff(int)));
        glayout->addWidget(m_piano[i],1,1);
        vlayout->addWidget(m_frame[i]);
        lbl->setBuddy(m_piano[i]);
        m_frame[i]->setVisible(false);
        m_action[i] = new KToggleAction(this);
        m_action[i]->setText(i18nc("@item:inmenu","Channel %1", i+1));
        connect(m_action[i], SIGNAL(triggered()), m_mapper, SLOT(map()));
        m_mapper->setMapping(m_action[i], i);
        chmenu->addAction(m_action[i]);
    }
    connect(m_mapper, SIGNAL(mapped(int)), SLOT(slotShowChannel(int)));
    setAutoSaveSettings("PlayerPianoWindow", true);
}

Pianola::~Pianola()
{ }

bool Pianola::queryClose()
{
    saveAutoSaveSettings();
    emit closed();
    return true;
}

void Pianola::allNotesOff()
{
    for (int ch = 0; ch < MIDI_CHANNELS; ++ch )
        if (m_action.at(ch) != NULL && m_action[ch]->isChecked())
            for( int n = 0; n < 128; ++n )
                if (m_piano.at(ch) != NULL)
                    m_piano[ch]->showNoteOff(n);
}

void Pianola::enableChannel(int channel, bool enable)
{
    if (!enable) {
        m_action[channel]->setChecked(false);
        m_frame[channel]->setVisible(false);
    }
    m_action[channel]->setEnabled(enable);
    update();
}

void Pianola::setNoteRange(int lowerNote, int upperNote)
{
    int octaveBase = lowerNote / 12;
    int numOctaves = upperNote / 12 - octaveBase + 1;
    for (int i = 0; i < MIDI_CHANNELS; ++i ) {
        m_piano[i]->setBaseOctave(octaveBase);
        m_piano[i]->setNumOctaves(numOctaves);
    }
}

void Pianola::slotShowChannel(int chan)
{
    m_frame[chan]->setVisible(m_action[chan]->isChecked());
    update();
}

void Pianola::slotNoteOn(int channel, int note, int vel)
{
    if (m_action[channel]->isChecked()) {
        if (vel == 0)
            m_piano[channel]->showNoteOff(note);
        else
            m_piano[channel]->showNoteOn(note);
	}
}

void Pianola::slotNoteOff(int channel, int note, int /*vel*/)
{
    if (m_action[channel]->isChecked())
        m_piano[channel]->showNoteOff(note);
}

void Pianola::playNoteOn(int note)
{
    PianoKeybd* p = static_cast<PianoKeybd*>(sender());
    if (p != NULL) {
        int channel = m_piano.indexOf(p);
        emit noteOn(channel, note, 110);
    }
}

void Pianola::playNoteOff(int note)
{
    PianoKeybd* p = static_cast<PianoKeybd*>(sender());
    if (p != NULL) {
        int channel = m_piano.indexOf(p);
        emit noteOff(channel, note, 0);
    }
}

void Pianola::showEvent( QShowEvent* /*event*/ )
{
    for (int i = 0; i < MIDI_CHANNELS; ++i ) {
        if (m_action[i]->isChecked())
            return;
    }
    for (int i = 0; i < MIDI_CHANNELS; ++i ) {
        if (m_action[i]->isEnabled()) {
            m_action[i]->setChecked(true);
            slotShowChannel(i);
            return;
        }
    }
}

void Pianola::slotShowAllChannels()
{
    for (int i = 0; i < MIDI_CHANNELS; ++i ) {
        if (m_action[i]->isEnabled() && !m_action[i]->isChecked()) {
            m_action[i]->setChecked(true);
            slotShowChannel(i);
        }
    }
}

void Pianola::slotHideAllChannels()
{
    for (int i = 0; i < MIDI_CHANNELS; ++i ) {
        if (m_action[i]->isEnabled() && m_action[i]->isChecked()) {
            m_action[i]->setChecked(false);
            slotShowChannel(i);
        }
    }
}

void Pianola::slotLabel(int channel, const QString& text)
{
    if (m_action[channel]->isEnabled())
        m_label[channel]->setText(text);
}
