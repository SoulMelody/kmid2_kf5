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

#ifndef KMID_PARTVIEW_H
#define KMID_PARTVIEW_H

#include <KF5/KMediaPlayer/kmediaplayer/view.h>

#ifndef KMIDPART_EXPORT
# if defined(kmid_part_EXPORTS) || defined(MAKE_KMID_PART_LIB)
#  define KMIDPART_EXPORT KDE_EXPORT
# else
#  define KMIDPART_EXPORT KDE_IMPORT
# endif
#endif


/**
 * KMidPartView is an implementation of the KMediaPlayer::View interface for
 * the KMidPart (MIDI/Karaoke player) component.
 *
 * KMediaPlayer::View defines four controls for the user interface:
 * the buttons play, pause and stop, and a seeker control. This class implements
 * the play/stop buttons as a single control, and the seeker as a slider. In
 * addition to them, there are also several controls hidden by default:
 * a pause button, two dial controls for volume and pitch transpose, and a
 * speed slider with a speed reset button. All of them can be shown or hidden
 * using the methods from KMediaPlayer::View with arguments calculated combining
 * values from the ExtraControl and Button enumerations.
 *
 * @see http://api.kde.org/4.x-api/kdelibs-apidocs/interfaces/kmediaplayer/
 */
class KMIDPART_EXPORT KMidPartView : public KMediaPlayer::View {
    Q_OBJECT
public:
    /**
     * Constructor
     * @param parent a parent widget
     */
    KMidPartView(QWidget *parent);

    /**
     * Destructor
     */
    virtual ~KMidPartView();

    /**
     * Controls that may appear in the UI (initially hidden). These values can
     * be used in addition to the Button enumeration.
     *
     * see @KMediaPlayer::View::Button
     */
    enum ExtraControl
    {
        /** Volume factor control */
        Volume = 16,
        /** Transpose control */
        Transpose = 32,
        /** Tempo (speed) factor */
        Tempo = 64
    };

    /**
     * Resets the time control range and position
     * @param totalTime song length in ticks
     */
    void resetTimePosition(qint64 totalTime);

    /**
     * Sets the time control position
     * @param tick song time in ticks
     */
    void setPosition(qint64 tick);

    /**
     * Changes the UI playback state
     * @param playing true if playing, false otherwise
     */
    void setPlayingState(bool playing);

private slots:
    void slotButtonsChanged(int);
    void slotPlayStop();
    void slotTimeSliderPressed();
    void slotTimeSlider(int);
    void slotVolumeSlider(int value);
    void slotPitchSlider(int value);
    void slotTempoSlider(int value);
    void slotTempoReset();

signals:
    /**
     * Emitted when the play control is activated
     */
    void play();

    /**
     * Emitted when the pause control is activated
     */
    void pause();

    /**
     * Emitted when the stop control is activated
     */
    void stop();

    /**
     * Emitted when the seeker position is changed
     * @param pos the new position of the seeker slider
     */
    void seek(int pos);

    /**
     * Emitted when the volume control is activated
     * @param volfactor volume factor: 0.0=min, 1.0=middle, 2.0=max
     */
    void volume(double volfactor);

    /**
     * Emitted when the transpose control is activated
     * @param pitch transpose factor: -12=min, 0=middle, +12=max
     */
    void transpose(int pitch);

    /**
     * Emitted when the speed control is activated
     * @param tempofactor speed factor: 0.5=min, 1.0=middle, 2.0=max
     */
    void speed(double tempofactor);

private:
    class ViewPrivate;
    ViewPrivate* d;
};

#endif /* KMID_PARTVIEW_H */
