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

#ifndef INCLUDED_PLAYER_H
#define INCLUDED_PLAYER_H

#include <QObject>
#include <playthread.h>
#include "song.h"

using namespace drumstick;

namespace KMid {

    class Player : public SequencerOutputThread
    {
        Q_OBJECT

    public:
        Player(MidiClient *seq, int portId);
        virtual ~Player();
        virtual bool hasNext();
        virtual SequencerEvent* nextEvent();
        virtual unsigned int getInitialPosition();
        virtual unsigned int getEchoResolution();

        void setSong(Song* s);
        void resetPosition();
        void setPosition(unsigned int pos);
        void setEchoResolution( const qint32 r );

    private:
        Song* m_song;
        SongIterator* m_songIterator;
        qint64 m_songPosition;
        qint32 m_echoResolution;
    };

}

#endif /*INCLUDED_PLAYER_H*/
