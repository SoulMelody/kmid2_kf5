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

#include "kmid_partview.h"

#include <KLocale>
#include <KMediaPlayer/View>

#include <QtWidgets/QSlider>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QDial>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>

class KMidPartView::ViewPrivate {
public:
    ViewPrivate():
        m_seeking(false),
        m_playing(false) {}
    virtual ~ViewPrivate() {}

    bool m_seeking;
    bool m_playing;
    QToolButton *m_playstop;
    QToolButton *m_pause;
    QSlider *m_seeker;
    QDial *m_volume;
    QDial *m_pitch;
    QToolButton *m_resetTempo;
    QSlider *m_tempo;
};

KMidPartView::KMidPartView(QWidget *parent) :
    KMediaPlayer::View(parent),
    d(new ViewPrivate())
{
    QVBoxLayout* vlayout = new QVBoxLayout(this);
    QHBoxLayout* toprow = new QHBoxLayout;
    vlayout->addLayout(toprow);

    d->m_playstop = new QToolButton(this);
    d->m_playstop->setIcon(QIcon("media-playback-start"));
    d->m_playstop->setWhatsThis(i18nc("@info:whatsthis","Player play/stop control"));
    d->m_playstop->setToolTip(i18nc("@info:tooltip player button","Play/Stop"));
    d->m_playstop->setVisible(true);
    toprow->addWidget(d->m_playstop);
    connect(d->m_playstop, SIGNAL(clicked()), SLOT(slotPlayStop()));

    d->m_pause = new QToolButton(this);
    d->m_pause->setIcon(QIcon("media-playback-pause"));
    d->m_pause->setWhatsThis(i18nc("@info:whatsthis","Player pause control"));
    d->m_pause->setToolTip(i18nc("@info:tooltip player button","Pause"));
    d->m_pause->setVisible(false);
    d->m_pause->setEnabled(false);
    toprow->addWidget(d->m_pause);
    connect(d->m_pause, SIGNAL(clicked()), SIGNAL(pause()));

    d->m_seeker = new QSlider(this);
    d->m_seeker->setRange(0, 10000);
    d->m_seeker->setOrientation(Qt::Horizontal);
    d->m_seeker->setWhatsThis(i18nc("@info:whatsthis","Time position control"));
    d->m_seeker->setToolTip(i18nc("@info:tooltip player button","Time position"));
    d->m_seeker->setTracking(false);
    toprow->addWidget(d->m_seeker);
    connect(d->m_seeker, SIGNAL(sliderPressed()), SLOT(slotTimeSliderPressed()));
    connect(d->m_seeker, SIGNAL(valueChanged(int)), SLOT(slotTimeSlider(int)));

    QHBoxLayout* botrow = new QHBoxLayout;
    vlayout->addLayout(botrow);

    d->m_volume = new QDial(this);
    d->m_volume->setVisible(false);
    d->m_volume->setFixedSize(32,32);
    d->m_volume->setWhatsThis(i18nc("@info:whatsthis","Volume factor control"));
    d->m_volume->setToolTip(i18nc("@info:tooltip player control","Volume factor"));
    d->m_volume->setMinimum(0);
    d->m_volume->setMaximum(200);
    d->m_volume->setValue(100);
    d->m_volume->setSingleStep(5);
    d->m_volume->setPageStep(10);
    d->m_volume->setTracking(false);
    d->m_volume->setNotchesVisible(true);
    botrow->addWidget(d->m_volume);
    connect(d->m_volume, SIGNAL(valueChanged(int)), SLOT(slotVolumeSlider(int)));

    d->m_pitch = new QDial(this);
    d->m_pitch->setVisible(false);
    d->m_pitch->setFixedSize(32,32);
    d->m_pitch->setWhatsThis(i18nc("@info:whatsthis","Pitch transpose control"));
    d->m_pitch->setToolTip(i18nc("@info:tooltip player control","Pitch transpose"));
    d->m_pitch->setMinimum(-12);
    d->m_pitch->setMaximum(12);
    d->m_pitch->setValue(0);
    d->m_pitch->setSingleStep(1);
    d->m_pitch->setPageStep(1);
    d->m_pitch->setTracking(false);
    d->m_pitch->setNotchesVisible(true);
    botrow->addWidget(d->m_pitch);
    connect(d->m_pitch, SIGNAL(valueChanged(int)), SLOT(slotPitchSlider(int)));

    d->m_resetTempo = new QToolButton(this);
    d->m_resetTempo->setVisible(false);
    d->m_resetTempo->setIcon(QIcon("player-time"));
    d->m_resetTempo->setWhatsThis(i18nc("@info:whatsthis","Speed factor reset"));
    d->m_resetTempo->setToolTip(i18nc("@info:tooltip player button","Speed reset"));
    botrow->addWidget(d->m_resetTempo);
    connect(d->m_resetTempo, SIGNAL(clicked()), SLOT(slotTempoReset()));

    d->m_tempo = new QSlider(this);
    d->m_tempo->setVisible(false);
    d->m_tempo->setOrientation(Qt::Horizontal);
    d->m_tempo->setWhatsThis(i18nc("@info:whatsthis","Speed factor control"));
    d->m_tempo->setToolTip(i18nc("@info:tooltip player control","Speed factor"));
    d->m_tempo->setMinimum(0);
    d->m_tempo->setMaximum(200);
    d->m_tempo->setValue(100);
    d->m_tempo->setTracking(false);
    botrow->addWidget(d->m_tempo);
    connect(d->m_tempo, SIGNAL(valueChanged(int)), SLOT(slotTempoSlider(int)));

    setButtons(Play | Stop | Seeker);
    connect(this, SIGNAL(buttonsChanged(int)), SLOT(slotButtonsChanged(int)));
}

KMidPartView::~KMidPartView()
{}

void KMidPartView::slotButtonsChanged(int buttons)
{
    d->m_playstop->setVisible( (buttons & (Play | Stop)) != 0 );
    d->m_pause->setVisible( (buttons & Pause) != 0 );
    d->m_seeker->setVisible( (buttons & Seeker) != 0 );
    d->m_volume->setVisible( (buttons & Volume) != 0 );
    d->m_pitch->setVisible( (buttons & Transpose) != 0 );
    d->m_resetTempo->setVisible( (buttons & Tempo) != 0 );
    d->m_tempo->setVisible( (buttons & Tempo) != 0 );
}

void KMidPartView::resetTimePosition(qint64 totalTime)
{
    d->m_seeker->setRange(0, totalTime);
    d->m_seeker->setSliderPosition(0);
    d->m_seeking = false;
}

void KMidPartView::setPosition(qint64 tick)
{
    if (!d->m_seeking)
        d->m_seeker->setSliderPosition(tick);
}

void KMidPartView::slotTimeSlider(int value)
{
    d->m_seeking = false;
    emit seek(value);
}

void KMidPartView::slotTimeSliderPressed()
{
    d->m_seeking = true;
}

void KMidPartView::slotPlayStop()
{
    if (d->m_playing)
        emit stop();
    else
        emit play();
}

void KMidPartView::setPlayingState(bool isPlaying)
{
    if (d->m_playing != isPlaying) {
        d->m_playing = isPlaying;
        if (isPlaying) {
            d->m_playstop->setIcon(QIcon("media-playback-stop"));
            d->m_pause->setEnabled(true);
        } else {
            d->m_playstop->setIcon(QIcon("media-playback-start"));
            d->m_pause->setEnabled(false);
        }
    }
}

void KMidPartView::slotVolumeSlider(int value)
{
    emit volume( value*0.01 );
}

void KMidPartView::slotPitchSlider(int value)
{
    emit transpose( value );
}

inline double tempoFactor(int sliderValue)
{
    return (sliderValue*sliderValue + 100.0*sliderValue + 20000.0) / 400.0;
}

void KMidPartView::slotTempoSlider(int value)
{
    emit speed( tempoFactor(value)/100.0 );
}

void KMidPartView::slotTempoReset()
{
    emit speed(1.0);
    d->m_tempo->setValue(100);
}
