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

#include "instrumentset.h"
#include <klocale.h>

InstrumentSet::InstrumentSet()
{
    m_names.reserve(128);
    reloadNames();
}

InstrumentSet::~InstrumentSet()
{ }

void InstrumentSet::reloadNames()
{
    m_names.clear();
    m_names.insert(0, i18nc("@item:inlistbox","Acoustic Grand Piano"));
    m_names.insert(1, i18nc("@item:inlistbox","Bright Acoustic Piano"));
    m_names.insert(2, i18nc("@item:inlistbox","Electric Grand Piano"));
    m_names.insert(3, i18nc("@item:inlistbox","Honky-Tonk"));
    m_names.insert(4, i18nc("@item:inlistbox","Rhodes Piano"));
    m_names.insert(5, i18nc("@item:inlistbox","Chorused Piano"));
    m_names.insert(6, i18nc("@item:inlistbox","Harpsichord"));
    m_names.insert(7, i18nc("@item:inlistbox","Clavinet"));
    m_names.insert(8, i18nc("@item:inlistbox","Celesta"));
    m_names.insert(9, i18nc("@item:inlistbox","Glockenspiel"));
    m_names.insert(10, i18nc("@item:inlistbox","Music Box"));
    m_names.insert(11, i18nc("@item:inlistbox","Vibraphone"));
    m_names.insert(12, i18nc("@item:inlistbox","Marimba"));
    m_names.insert(13, i18nc("@item:inlistbox","Xylophone"));
    m_names.insert(14, i18nc("@item:inlistbox","Tubular Bells"));
    m_names.insert(15, i18nc("@item:inlistbox","Dulcimer"));
    m_names.insert(16, i18nc("@item:inlistbox","Hammond Organ"));
    m_names.insert(17, i18nc("@item:inlistbox","Percussive Organ"));
    m_names.insert(18, i18nc("@item:inlistbox","Rock Organ"));
    m_names.insert(19, i18nc("@item:inlistbox","Church Organ"));
    m_names.insert(20, i18nc("@item:inlistbox","Reed Organ"));
    m_names.insert(21, i18nc("@item:inlistbox","Accordion"));
    m_names.insert(22, i18nc("@item:inlistbox","Harmonica"));
    m_names.insert(23, i18nc("@item:inlistbox","Tango Accordion"));
    m_names.insert(24, i18nc("@item:inlistbox","Acoustic Guitar (Nylon)"));
    m_names.insert(25, i18nc("@item:inlistbox","Acoustic Guitar (Steel)"));
    m_names.insert(26, i18nc("@item:inlistbox","Electric Guitar (Jazz)"));
    m_names.insert(27, i18nc("@item:inlistbox","Electric Guitar (Clean)"));
    m_names.insert(28, i18nc("@item:inlistbox","Electric Guitar (Muted)"));
    m_names.insert(29, i18nc("@item:inlistbox","Overdriven Guitar"));
    m_names.insert(30, i18nc("@item:inlistbox","Distortion Guitar"));
    m_names.insert(31, i18nc("@item:inlistbox","Guitar Harmonics"));
    m_names.insert(32, i18nc("@item:inlistbox","Acoustic Bass"));
    m_names.insert(33, i18nc("@item:inlistbox","Electric Bass (Finger)"));
    m_names.insert(34, i18nc("@item:inlistbox","Electric Bass (Pick)"));
    m_names.insert(35, i18nc("@item:inlistbox","Fretless Bass"));
    m_names.insert(36, i18nc("@item:inlistbox","Slap Bass 1"));
    m_names.insert(37, i18nc("@item:inlistbox","Slap Bass 2"));
    m_names.insert(38, i18nc("@item:inlistbox","Synth Bass 1"));
    m_names.insert(39, i18nc("@item:inlistbox","Synth Bass 2"));
    m_names.insert(40, i18nc("@item:inlistbox","Violin"));
    m_names.insert(41, i18nc("@item:inlistbox","Viola"));
    m_names.insert(42, i18nc("@item:inlistbox","Cello"));
    m_names.insert(43, i18nc("@item:inlistbox","Contrabass"));
    m_names.insert(44, i18nc("@item:inlistbox","Tremolo Strings"));
    m_names.insert(45, i18nc("@item:inlistbox","Pizzicato Strings"));
    m_names.insert(46, i18nc("@item:inlistbox","Orchestral Harp"));
    m_names.insert(47, i18nc("@item:inlistbox","Timpani"));
    m_names.insert(48, i18nc("@item:inlistbox","String Ensemble 1"));
    m_names.insert(49, i18nc("@item:inlistbox","String Ensemble 2"));
    m_names.insert(50, i18nc("@item:inlistbox","Synth Strings 1"));
    m_names.insert(51, i18nc("@item:inlistbox","Synth Strings 2"));
    m_names.insert(52, i18nc("@item:inlistbox","Choir Aahs"));
    m_names.insert(53, i18nc("@item:inlistbox","Voice Oohs"));
    m_names.insert(54, i18nc("@item:inlistbox","Synth Voice"));
    m_names.insert(55, i18nc("@item:inlistbox blow, stroke, impact","Orchestra Hit"));
    m_names.insert(56, i18nc("@item:inlistbox","Trumpet"));
    m_names.insert(57, i18nc("@item:inlistbox","Trombone"));
    m_names.insert(58, i18nc("@item:inlistbox","Tuba"));
    m_names.insert(59, i18nc("@item:inlistbox","Muted Trumpet"));
    m_names.insert(60, i18nc("@item:inlistbox","French Horn"));
    m_names.insert(61, i18nc("@item:inlistbox","Brass Section"));
    m_names.insert(62, i18nc("@item:inlistbox","Synth Brass 1"));
    m_names.insert(63, i18nc("@item:inlistbox","Synth Brass 2"));
    m_names.insert(64, i18nc("@item:inlistbox","Soprano Sax"));
    m_names.insert(65, i18nc("@item:inlistbox","Alto Sax"));
    m_names.insert(66, i18nc("@item:inlistbox","Tenor Sax"));
    m_names.insert(67, i18nc("@item:inlistbox","Baritone Sax"));
    m_names.insert(68, i18nc("@item:inlistbox","Oboe"));
    m_names.insert(69, i18nc("@item:inlistbox","English Horn"));
    m_names.insert(70, i18nc("@item:inlistbox","Bassoon"));
    m_names.insert(71, i18nc("@item:inlistbox","Clarinet"));
    m_names.insert(72, i18nc("@item:inlistbox","Piccolo"));
    m_names.insert(73, i18nc("@item:inlistbox","Flute"));
    m_names.insert(74, i18nc("@item:inlistbox woodwind musical instrument. http://en.wikipedia.org/wiki/Recorder","Recorder"));
    m_names.insert(75, i18nc("@item:inlistbox","Pan Flute"));
    m_names.insert(76, i18nc("@item:inlistbox","Blown Bottle"));
    m_names.insert(77, i18nc("@item:inlistbox","Shakuhachi"));
    m_names.insert(78, i18nc("@item:inlistbox","Whistle"));
    m_names.insert(79, i18nc("@item:inlistbox","Ocarina"));
    m_names.insert(80, i18nc("@item:inlistbox","Lead 1 - Square Wave"));
    m_names.insert(81, i18nc("@item:inlistbox","Lead 2 - Saw Tooth"));
    m_names.insert(82, i18nc("@item:inlistbox","Lead 3 - Calliope"));
    m_names.insert(83, i18nc("@item:inlistbox","Lead 4 - Chiflead"));
    m_names.insert(84, i18nc("@item:inlistbox","Lead 5 - Charang"));
    m_names.insert(85, i18nc("@item:inlistbox","Lead 6 - Voice"));
    m_names.insert(86, i18nc("@item:inlistbox","Lead 7 - Fifths"));
    m_names.insert(87, i18nc("@item:inlistbox","Lead 8 - Bass+Lead"));
    m_names.insert(88, i18nc("@item:inlistbox","Pad 1 - New Age"));
    m_names.insert(89, i18nc("@item:inlistbox","Pad 2 - Warm"));
    m_names.insert(90, i18nc("@item:inlistbox","Pad 3 - Polysynth"));
    m_names.insert(91, i18nc("@item:inlistbox","Pad 4 - Choir"));
    m_names.insert(92, i18nc("@item:inlistbox","Pad 5 - Bow"));
    m_names.insert(93, i18nc("@item:inlistbox","Pad 6 - Metallic"));
    m_names.insert(94, i18nc("@item:inlistbox","Pad 7 - Halo"));
    m_names.insert(95, i18nc("@item:inlistbox","Pad 8 - Sweep"));
    m_names.insert(96, i18nc("@item:inlistbox","FX 1 - Rain"));
    m_names.insert(97, i18nc("@item:inlistbox","FX 2 - Soundtrack"));
    m_names.insert(98, i18nc("@item:inlistbox","FX 3 - Crystal"));
    m_names.insert(99, i18nc("@item:inlistbox","FX 4 - Atmosphere"));
    m_names.insert(100, i18nc("@item:inlistbox","FX 5 - Brightness"));
    m_names.insert(101, i18nc("@item:inlistbox","FX 6 - Goblins"));
    m_names.insert(102, i18nc("@item:inlistbox","FX 7 - Echoes"));
    m_names.insert(103, i18nc("@item:inlistbox","FX 8 - Sci-fi"));
    m_names.insert(104, i18nc("@item:inlistbox","Sitar"));
    m_names.insert(105, i18nc("@item:inlistbox","Banjo"));
    m_names.insert(106, i18nc("@item:inlistbox","Shamisen"));
    m_names.insert(107, i18nc("@item:inlistbox","Koto"));
    m_names.insert(108, i18nc("@item:inlistbox","Kalimba"));
    m_names.insert(109, i18nc("@item:inlistbox","Bagpipe"));
    m_names.insert(110, i18nc("@item:inlistbox","Fiddle"));
    m_names.insert(111, i18nc("@item:inlistbox","Shannai"));
    m_names.insert(112, i18nc("@item:inlistbox","Tinkle Bell"));
    m_names.insert(113, i18nc("@item:inlistbox","Agogo"));
    m_names.insert(114, i18nc("@item:inlistbox","Steel Drum"));
    m_names.insert(115, i18nc("@item:inlistbox","Wood Block"));
    m_names.insert(116, i18nc("@item:inlistbox","Taiko Drum"));
    m_names.insert(117, i18nc("@item:inlistbox","Melodic Tom"));
    m_names.insert(118, i18nc("@item:inlistbox","Synth Drum"));
    m_names.insert(119, i18nc("@item:inlistbox","Reverse Cymbal"));
    m_names.insert(120, i18nc("@item:inlistbox","Guitar Fret Noise"));
    m_names.insert(121, i18nc("@item:inlistbox","Breath Noise"));
    m_names.insert(122, i18nc("@item:inlistbox","Seashore"));
    m_names.insert(123, i18nc("@item:inlistbox","Bird Tweet"));
    m_names.insert(124, i18nc("@item:inlistbox","Telephone"));
    m_names.insert(125, i18nc("@item:inlistbox","Helicopter"));
    m_names.insert(126, i18nc("@item:inlistbox","Applause"));
    m_names.insert(127, i18nc("@item:inlistbox","Gunshot"));

    m_numbers.clear();
    for(int i=1; i<129; ++i)
        m_numbers.append(i18nc("@item:inlistbox","Percussion %1", i));
}

QString InstrumentSet::name(int patch, bool percussion) const
{
    if (percussion)
        return m_numbers.value(patch);
    return m_names.value(patch);
}

QStringList InstrumentSet::names(bool percussion) const
{
    if (percussion)
        return m_numbers;
    return m_names.toList();
}
