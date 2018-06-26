/*
    midimapper.cpp  - midi mapper object implementation

    Copyright (C) 1997-2000  Antonio Larrosa Jimenez <larrosa@kde.org>
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

#include "midimapper.h"
#include <QMap>
#include <KConfig>
#include <KConfigGroup>
#include <KDebug>

namespace KMid {

    class MidiMapper::MidiMapperPrivate {
    public:
        MidiMapperPrivate()
        {
            init();
        }

        void init()
        {
            int i;
            m_ok = false;
            m_forceDrumsPatch = -1;
            m_mapExpressionToVolumeEvents = false;
            m_mapPitchBender = false;
            m_pitchBenderRatio = 4096;
            m_filename.clear();
            QString notes[] =
                {"C ","C#","D ","D#","E ","F ","F#","G ","G#","A ","A#","B "};
            for (i = 0; i < MIDI_CHANNELS; ++i) {
                m_channelmap[i] = i;
            }
            for (i = 0; i < 128; ++i) {
                m_patchmap[i] = i;
                m_keymap[i] = i;
                m_forcedkey[i] = -1;
                m_keyNamesList << notes[i%12] + QString::number(i/12);
            }
            m_patchNamesList
                << "AcusticPiano"
                << "BrightPiano"
                << "SynthPiano"
                << "HonkyTonky"
                << "ElectricPiano1"
                << "ElectricPiano2"
                << "Harpsichord"
                << "Clavinet"
                << "Celeste"
                << "Glockenspiel"
                << "Musicbox"
                << "Vibes"
                << "Marimba"
                << "Xylophon"
                << "TubeBell"
                << "Santur"
                << "HomeOrg"
                << "PercussionOrg"
                << "RockOrg"
                << "ChurchOrg"
                << "ReedOrg"
                << "Accordion"
                << "Harmonica"
                << "Concrtna"
                << "NylonGuitar"
                << "AcusticGuitar"
                << "JazzGuitar"
                << "CleanGuitar"
                << "MuteGuitar"
                << "OdGuitar"
                << "DistortionGuit"
                << "GtrHarm"
                << "AcusticBass"
                << "FingerBass"
                << "PickBass"
                << "FretlessBass"
                << "SlapBass1"
                << "SlapBass2"
                << "SynthBass1"
                << "SynthBass2"
                << "Violin"
                << "Viola"
                << "Cello"
                << "Contrabajo"
                << "Marcato"
                << "Pizzicato"
                << "Harp"
                << "Timpani"
                << "Marcato"
                << "SlowStrings"
                << "SynthStrings1"
                << "SynthStrings2"
                << "Choir"
                << "Doo"
                << "Voices"
                << "OrchestraHit"
                << "Trumpet"
                << "Trombone"
                << "Tuba"
                << "MuteTrumpet"
                << "FrenchHorn"
                << "HitBrass"
                << "SynthBrass1"
                << "SynthBrass2"
                << "SopranoSax"
                << "AltoSax"
                << "TenorSax"
                << "BaritoneSax"
                << "Oboe"
                << "EnglishHorn"
                << "Bassoon"
                << "Clarinet"
                << "Piccolo"
                << "Flute"
                << "Recorder"
                << "WoodFlute"
                << "Bottle"
                << "Shakazul"
                << "Whistle"
                << "Ocarina"
                << "SquareWave"
                << "SawWave"
                << "Calliope"
                << "Chiflead"
                << "Charang"
                << "VoxLead"
                << "Lead5th"
                << "BassLead"
                << "Fantasia"
                << "WarmPad"
                << "Polysyn"
                << "Ghostie"
                << "BowGlass"
                << "MetalPad"
                << "HaloPad"
                << "Sweeper"
                << "Aurora"
                << "SoundTrack"
                << "Crystal"
                << "Atmosphear"
                << "FreshAir"
                << "Unicorn"
                << "Sweeper"
                << "StarTrak"
                << "Sitar"
                << "Banjo"
                << "Shamisen"
                << "Koto"
                << "Kalimba"
                << "BagPipes"
                << "Fiddle"
                << "Shannai"
                << "Carillon"
                << "Agogo"
                << "SteelDrm"
                << "WoodBlk"
                << "Taiko"
                << "Toms"
                << "Syntom"
                << "RevCymbal"
                << "Fx-Fret"
                << "Fx-Blow"
                << "Seashore"
                << "Jungle"
                << "Telephon"
                << "Helicopter"
                << "Applause"
                << "Gunshot";
        }

        void load(const QString &fileName)
        {
            KConfig cfg(fileName, KConfig::CascadeConfig);//, "appdata"
            init();
            m_filename = cfg.name();

            KConfigGroup grp = cfg.group("PATCHMAP");
            if (grp.exists()) {
                QMapIterator<QString,QString> it(grp.entryMap());
                while (it.hasNext()) {
                    it.next();
                    bool ok;
                    int idx = m_patchNamesList.indexOf(it.key());
                    int val = it.value().toInt(&ok);
                    if (idx > -1 && ok)
                        m_patchmap[idx] = val;
                    else
                        kWarning() << it.key() << "=" << it.value();
                }
            }

            grp = cfg.group("FIXEDKEY");
            if (grp.exists()) {
                QMapIterator<QString,QString> it(grp.entryMap());
                while (it.hasNext()) {
                    it.next();
                    bool ok1, ok2;
                    int idx = it.key().toInt(&ok1);
                    int val = it.value().toInt(&ok2);
                    if (ok1 && ok2)
                        m_forcedkey[idx] = val;
                    else
                        kWarning() << it.key() << "=" << it.value();
                }
            }

            grp = cfg.group("KEYMAP");
            if (grp.exists()) {
                QMapIterator<QString,QString> it(grp.entryMap());
                while (it.hasNext()) {
                    it.next();
                    bool ok;
                    int idx = m_keyNamesList.indexOf(it.key().left(4).trimmed());
                    int val = it.value().toInt(&ok);
                    if (idx > -1 && ok)
                        m_keymap[idx] = val;
                    else
                        kWarning() << it.key() << "=" << it.value();
                }
            }

            grp = cfg.group("CHANNELMAP");
            if (grp.exists()) {
                QMapIterator<QString,QString> it(grp.entryMap());
                while (it.hasNext()) {
                    it.next();
                    bool ok1, ok2;
                    int idx = it.key().toInt(&ok1);
                    ok1 &= (idx >= 0 && idx < MIDI_CHANNELS);
                    int val = it.value().toInt(&ok2);
                    ok2 &= (val >= 0 && val < MIDI_CHANNELS);
                    if (ok1 && ok2)
                        m_channelmap[idx] = val;
                    else
                        kWarning() << it.key() << "=" << it.value();
                }
            }

            grp = cfg.group("OPTIONS");
            if (grp.exists()) {
                m_forceDrumsPatch = grp.readEntry("ForceDrumsPatch", -1);
                m_mapPitchBender = grp.hasKey("PitchBenderRatio");
                if (m_mapPitchBender)
                    m_pitchBenderRatio =  grp.readEntry("PitchBenderRatio", 4096);
                m_mapExpressionToVolumeEvents =
                        grp.readEntry("MapExpressionToVolumeEvents", false);
            }
            m_ok = true;
        }

        QStringList m_patchNamesList;

        QStringList m_keyNamesList;

        /**
         * Object statusOK
         */
        bool m_ok;

        /**
         * @internal
         * Channel map translator
         */
        uchar m_channelmap[MIDI_CHANNELS];

        /**
         * @internal
         * It's -1 if the drums channel doesn't have a forced patch,
         * else indicates the patch to force in the channel.
         */
        int m_forceDrumsPatch;

        /**
         * @internal
         * Patch map translator
         */
        uchar m_patchmap[128];

        /**
         * @internal
         * Key map translator (for drums)
         */
        uchar m_keymap[128];

        /**
         * @internal
         * Forced Key by patch
         */
        int m_forcedkey[128];

        /**
         * @internal
         * Stores the name of the file from which the map was loaded
         */
        QString m_filename;

        /**
         * @internal
         * Simulate expression events with volume events
         */
        bool m_mapExpressionToVolumeEvents;

        /**
         * @internal
         * Map or not the Pitch Bender using m_pitchBenderRatio
         */
        bool m_mapPitchBender;

        /**
         * @internal
         * Indicates the ratio between the standard and the synthesizer's pitch
         * bender engine. The number sent to the synth is multiplied by this
         * and dividied by 4096. Thus if PitchBenderRatio is 4096, the synth's
         * pitch bender works as the standard one
         */
        int m_pitchBenderRatio;
    };

    MidiMapper::MidiMapper() :
        d(new MidiMapperPrivate)
    { }

    /**
     * Destructor.
     */
    MidiMapper::~MidiMapper()
    {
        delete d;
    }

    QString MidiMapper::fileName()
    {
        return d->m_filename;
    }

    void MidiMapper::loadFile(const QString &fileName)
    {
        d->load( fileName );
    }

    void MidiMapper::clear()
    {
        d->init();
    }

    uchar MidiMapper::channel(uchar chn)
    {
        return d->m_channelmap[chn];
    }

    uchar MidiMapper::controller(uchar ctl)
    {
        if ( d->m_mapExpressionToVolumeEvents )
            switch ( ctl ) {
            case MIDI_CTL_MSB_EXPRESSION:
                return MIDI_CTL_MSB_MAIN_VOLUME;
                break;
            case MIDI_CTL_LSB_EXPRESSION:
                return MIDI_CTL_LSB_MAIN_VOLUME;
                break;
            }
        return ctl;
    }

    uchar MidiMapper::key(uchar chn, uchar pgm, uchar note)
    {
        if (chn == MIDI_GM_DRUM_CHANNEL)
            return d->m_keymap[note];
        else if (d->m_forcedkey[pgm] > -1)
                return d->m_forcedkey[pgm];
        return note;
    }

    uchar MidiMapper::patch(uchar chn, uchar pgm)
    {
        if ( chn == MIDI_GM_DRUM_CHANNEL && d->m_forceDrumsPatch > -1)
            return d->m_forceDrumsPatch;
        return d->m_patchmap[pgm];
    }

    int MidiMapper::pitchBender(int value)
    {
        if (d->m_mapPitchBender)
            return value * d->m_pitchBenderRatio / 4096;
        return value;
    }

    bool MidiMapper::isOK()
    {
        return d->m_ok;
    }

}
