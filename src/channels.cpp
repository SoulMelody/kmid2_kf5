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

#include "channels.h"
#include "vumeter.h"

#include <QSignalMapper>
#include <QGridLayout>
#include <QLabel>
#include <QToolButton>
#include <QPixmap>
#include <QIcon>
#include <QTimerEvent>
#include <KLocale>
#include <KComboBox>
#include <KLineEdit>

Channels::Channels( QWidget* parent ) :
    KMainWindow(parent),
    m_timerId(0),
    m_volumeFactor(1.0)
{
    setObjectName("ChannelsWindow");
    setAttribute(Qt::WA_DeleteOnClose, false);
    setCaption(i18nc("@title:window","MIDI Channels"));
    m_soloMapper = new QSignalMapper(this);
    m_muteMapper = new QSignalMapper(this);
    m_patchMapper = new QSignalMapper(this);
    m_lockMapper = new QSignalMapper(this);
    m_nameMapper = new QSignalMapper(this);
    QGridLayout *layout = new QGridLayout;
    layout->setHorizontalSpacing(10);
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    centralWidget->setLayout(layout);
    QLabel* lbl = new QLabel(this);
    lbl->setText(i18nc("MIDI channel number", "Channel"));
    layout->addWidget(lbl, 0, 0, 1, 2);
    lbl = new QLabel(this);
    lbl->setText(i18nc("Sound muting", "Mute"));
    layout->addWidget(lbl, 0, 2);
    lbl = new QLabel(this);
    lbl->setText(i18nc("Soloist channel", "Solo"));
    layout->addWidget(lbl, 0, 3);
    lbl = new QLabel(this);
    lbl->setText(i18nc("Channel volume level", "Level"));
    layout->addWidget(lbl, 0, 4);
    lbl = new QLabel(this);
    lbl->setText(i18nc("Instrument locked", "Lock"));
    layout->addWidget(lbl, 0, 5);
    lbl = new QLabel(this);
    lbl->setText(i18nc("MIDI program/patch, predefined instrument","Patch (sound setting)"));
    layout->addWidget(lbl, 0, 6);
    layout->setColumnStretch(1,1);
    QSize ledSize(12,12);
    QPixmap pixMuteOn(ledSize);
    QPixmap pixMuteOff(ledSize);
    pixMuteOn.fill(Qt::red);
    pixMuteOff.fill(QColor(90,0,0));
    QIcon muteIcon;
    muteIcon.addPixmap(pixMuteOn, QIcon::Normal, QIcon::On);
    muteIcon.addPixmap(pixMuteOff,QIcon::Normal, QIcon::Off);
    QPixmap pixSoloOn(ledSize);
    QPixmap pixSoloOff(ledSize);
    pixSoloOn.fill(Qt::green);
    pixSoloOff.fill(QColor(0,90,0));
    QIcon soloIcon;
    soloIcon.addPixmap(pixSoloOn, QIcon::Normal, QIcon::On);
    soloIcon.addPixmap(pixSoloOff,QIcon::Normal, QIcon::Off);
    QIcon locked("object-locked");
    QIcon unlocked("object-unlocked");
    QIcon lockIcon;
    QSize lockSize(16,16);
    lockIcon.addPixmap(locked.pixmap(lockSize), QIcon::Normal, QIcon::On);
    lockIcon.addPixmap(unlocked.pixmap(lockSize), QIcon::Normal, QIcon::Off);
    for (int i = 0; i < MIDI_CHANNELS; ++i) {
        int row = i + 1;
        lbl = new QLabel(this);
        lbl->setNum(row);
        layout->addWidget(lbl, row, 0, Qt::AlignRight | Qt::AlignVCenter);
        m_name[i] = new KLineEdit(this);
        layout->addWidget(m_name[i], row, 1);
        connect( m_name[i], SIGNAL(editingFinished()), m_nameMapper, SLOT(map()) );
        m_nameMapper->setMapping( m_name[i], i );
        m_mute[i] = new QToolButton(this);
        m_mute[i]->setCheckable(true);
        m_mute[i]->setIcon(muteIcon);
        layout->addWidget(m_mute[i], row, 2);
        connect( m_mute[i], SIGNAL(clicked()), m_muteMapper, SLOT(map()) );
        m_muteMapper->setMapping( m_mute[i], i);
        m_solo[i] = new QToolButton(this);
        m_solo[i]->setCheckable(true);
        m_solo[i]->setIcon(soloIcon);
        layout->addWidget(m_solo[i], row, 3);
        connect( m_solo[i], SIGNAL(clicked()), m_soloMapper, SLOT(map()) );
        m_soloMapper->setMapping( m_solo[i], i);
        m_vumeter[i] = new Vumeter(this);
        layout->addWidget(m_vumeter[i], row, 4);
        m_lock[i] = new QToolButton(this);
        m_lock[i]->setCheckable(true);
        m_lock[i]->setIcon(lockIcon);
        layout->addWidget(m_lock[i], row, 5);
        connect( m_lock[i], SIGNAL(clicked()), m_lockMapper, SLOT(map()) );
        m_lockMapper->setMapping( m_lock[i], i);
        m_patch[i] = new KComboBox(this);
        m_patch[i]->addItems(m_instSet.names(i == MIDI_GM_DRUM_CHANNEL));
        layout->addWidget(m_patch[i], row, 6);
        connect( m_patch[i], SIGNAL(activated(int)), m_patchMapper, SLOT(map()) );
        m_patchMapper->setMapping(m_patch[i], i);
        lbl->setBuddy(m_patch[i]);
        m_voices[i] = 0;
        m_level[i] = 0.0;
        m_factor[i] = m_volumeFactor;
    }
    connect(m_muteMapper, SIGNAL(mapped(int)), SLOT(slotMuteChannel(int)));
    connect(m_soloMapper, SIGNAL(mapped(int)), SLOT(slotSoloChannel(int)));
    connect(m_patchMapper, SIGNAL(mapped(int)), SLOT(slotPatchChanged(int)));
    connect(m_lockMapper, SIGNAL(mapped(int)), SLOT(slotLockChannel(int)));
    connect(m_nameMapper, SIGNAL(mapped(int)), SLOT(slotNameChannel(int)));
    setAutoSaveSettings("ChannelsWindow", true);
}

Channels::~Channels()
{ }

bool Channels::queryClose()
{
    saveAutoSaveSettings();
    emit closed();
    return true;
}

void Channels::enableChannel(int channel, bool enable)
{
    m_mute[channel]->setChecked(false);
    m_mute[channel]->setEnabled(enable);
    m_solo[channel]->setChecked(false);
    m_solo[channel]->setEnabled(enable);
    m_vumeter[channel]->setValue(0);
    m_vumeter[channel]->setEnabled(enable);
    if (!m_lock[channel]->isChecked())
        m_patch[channel]->setCurrentIndex(-1);
    m_patch[channel]->setEnabled(enable);
    m_name[channel]->clear();
    m_name[channel]->setEnabled(enable);
    m_lock[channel]->setEnabled(enable);
}

void Channels::slotDisableAllChannels()
{
    for ( int channel = 0; channel < MIDI_CHANNELS; ++channel )
        enableChannel(channel, false);
}

void Channels::slotEnableAllChannels()
{
    for ( int channel = 0; channel < MIDI_CHANNELS; ++channel )
        enableChannel(channel, true);
}

void Channels::slotPatch(int channel, int value)
{
    if (m_lock[channel]->isChecked())
        return;
    m_patch[channel]->setCurrentIndex(value);
}

void Channels::slotNoteOn(int channel, int /*note*/, int vel)
{
    qreal r = vel / 127.0 * m_factor[channel];
    if (r > 0.0) {
        m_level[channel] = r;
        m_voices[channel] += 1;
    } else
        m_voices[channel] -= 1;

    if (m_timerId == 0)
        m_timerId = startTimer(200);
}

void Channels::slotNoteOff(int channel, int /*note*/, int /*vel*/)
{
    m_voices[channel] -= 1;
}

void Channels::slotPatchChanged(int channel)
{
    int p = m_patch[channel]->currentIndex();
    if (p > -1)
        emit patch(channel, p);
}

void Channels::slotMuteChannel(int channel)
{
    emit mute(channel, m_mute[channel]->isChecked());
}

void Channels::slotSoloChannel(int channel)
{
    bool enable = m_solo[channel]->isChecked();
    for ( int ch = 0; ch < MIDI_CHANNELS; ++ch )
        if (channel != ch) {
            m_solo[ch]->setChecked(false);
            m_factor[ch] = enable ? m_volumeFactor * 0.5 : m_volumeFactor;
            emit volume(ch, m_factor[ch]);
        }
    m_factor[channel] = m_volumeFactor;
    emit volume(channel, m_volumeFactor);
}

void Channels::timerEvent(QTimerEvent *event)
{
    if (m_timerId == event->timerId()) {
        qreal v;
        bool kill = true;
        for ( int ch = 0; ch < MIDI_CHANNELS; ++ch ) {
            if (m_voices[ch] > 0) {
                v = m_level[ch];
                m_vumeter[ch]->setValue(v);
            } else {
                v = m_vumeter[ch]->decay(10);
                m_level[ch] = v;
                m_voices[ch] = 0;
            }
            kill &= (v <= 0.0);
        }
        if (kill) {
            killTimer(m_timerId);
            m_timerId = 0;
        }
    }
}

qreal Channels::volumeFactor()
{
    return m_volumeFactor;
}

void Channels::setVolumeFactor(qreal factor)
{
    m_volumeFactor = factor;
    for ( int ch = 0; ch < MIDI_CHANNELS; ++ch ) {
        m_solo[ch]->setChecked(false);
        m_factor[ch] = m_volumeFactor;
    }
}

void Channels::allNotesOff()
{
    for ( int ch = 0; ch < MIDI_CHANNELS; ++ch )
        m_voices[ch] = 0;
}

void Channels::setChannelName(int channel, const QString& name)
{
    m_name[channel]->setText(name);
}

QString Channels::channelName(int channel) const
{
    return m_name[channel]->text();
}

void Channels::slotLockChannel(int channel)
{
    emit lock(channel, m_lock[channel]->isChecked());
}

bool Channels::isChannelLocked(int channel) const
{
    return m_lock[channel]->isChecked();
}

bool Channels::isChannelMuted(int channel) const
{
    return m_mute[channel]->isChecked();
}

void Channels::setMuteChannel(int channel, bool mute)
{
    m_mute[channel]->setChecked(mute);
}

bool Channels::isChannelSoloed(int channel) const
{
    return m_solo[channel]->isChecked();
}

void Channels::setSoloChannel(int channel, bool solo)
{
    m_solo[channel]->setChecked(solo);
}

int Channels::channelPatch(int channel) const
{
    return m_patch[channel]->currentIndex();
}

void Channels::setPatchChannel(int channel, int patch)
{
    m_patch[channel]->setCurrentIndex(patch);
}

void Channels::setLockChannel(int channel, bool lock)
{
    m_lock[channel]->setChecked(lock);
}

void Channels::slotNameChannel(int channel)
{
    emit name(channel, m_name[channel]->text());
}
