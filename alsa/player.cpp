/*
    KMid Backend using the ALSA Sequencer
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

#include "player.h"
#include "song.h"

namespace KMid {

    Player::Player(MidiClient *seq, int portId)
        : SequencerOutputThread(seq, portId),
        m_song(0),
        m_songIterator(0),
        m_songPosition(0),
        m_echoResolution(0)
    {
        moveToThread(this);
    }

    Player::~Player()
    {
        if (isRunning())
            stop();
        if (m_songIterator != NULL) {
            delete m_songIterator;
        }
    }

    void Player::setSong(Song* s)
    {
        m_song = s;
        if (m_song != NULL) {
            if (m_songIterator != NULL) {
                delete m_songIterator;
            }
            m_songIterator = new SongIterator(*m_song);
            if (m_echoResolution == 0)
                m_echoResolution = m_song->getDivision() / 12;
            resetPosition();
        }
    }

    void Player::resetPosition()
    {
        if ((m_song != NULL) && (m_songIterator != NULL)) {
            m_songIterator->toFront();
            m_songPosition = 0;
        }
    }

    void Player::setPosition(unsigned int pos)
    {
        m_songPosition = pos;
        m_songIterator->toFront();
        while (m_songIterator->hasNext() &&
              (m_songIterator->next()->getTick() < pos)) {};
        if (m_songIterator->hasPrevious())
            m_songIterator->previous();
    }

    bool Player::hasNext()
    {
        return m_songIterator->hasNext();
    }

    SequencerEvent* Player::nextEvent()
    {
        return m_songIterator->next();
    }

    unsigned int Player::getInitialPosition()
    {
        return m_songPosition;
    }

    unsigned int Player::getEchoResolution()
    {
        return m_echoResolution;
    }

    void Player::setEchoResolution( const qint32 r )
    {
        m_echoResolution = r;
    }

}
