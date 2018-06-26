/*
    KMid Mac OSX Backend
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

#include "dlssoftsynth.h"
#include "macmidioutput.h"
#include "settings.h"
#include <KDebug>

#define PRETTY_NAME "DLS Synth"

namespace KMid {

    void SynthMIDIReadProc( const MIDIPacketList *pktlist,
                            void *refCon, void *connRefCon )
    {
        DLSSoftSynth *obj = NULL;
        if (refCon != NULL)
            obj = reinterpret_cast<DLSSoftSynth *>(refCon);
        if (obj != NULL)
            obj->sendEvents(pktlist);
    }

    DLSSoftSynth::DLSSoftSynth( Settings* settings, QObject *parent ) :
        QObject(parent),
        m_graph(0),
        m_synthNode(0),
        m_outputNode(0),
        m_endpoint(0),
        m_settings(settings)
    {
        m_settingsNames << "exec_dls";
        m_settingsNames << "soundfont_dls";
        m_settingsNames << "default_dls";
        m_settingsNames << "reverb_dls";
    }

    DLSSoftSynth::~DLSSoftSynth()
    {
        OSStatus result = noErr;
        if (m_endpoint != NULL) {
            result = MIDIEndpointDispose(m_endpoint);
            if (result != noErr)
                kDebug() << "MIDIEndpointDispose() err:" << result;
        }
        terminate();
    }

    void DLSSoftSynth::registerStatus(const QString& context, const long status)
    {
        if (status != noErr)
            m_messages << QString("%1 err:%2").arg(context).arg(status);
    }

    void DLSSoftSynth::initialize( MacMIDIOutput *output )
    {
        OSStatus result = noErr;
        result = MIDIDestinationCreate ( output->client(),
            CFSTR(PRETTY_NAME), SynthMIDIReadProc,
            (void*) this, &m_endpoint );
        registerStatus("MIDIDestinationCreate", result);
    }

    bool DLSSoftSynth::isInitialized() const
    {
        return (m_endpoint != 0);
    }

    void inline midiEvent(AudioUnit synth, Byte* msg, int msglen)
    {
        if ( msg[0] == MIDI_STATUS_SYSEX )
            MusicDeviceSysEx ( synth, msg, msglen );
        else
            MusicDeviceMIDIEvent ( synth, msg[0], msg[1], msg[2], 0 );
    }

    void DLSSoftSynth::sendEvents(const MIDIPacketList *packets)
    {
        int i, j, size = 0;
        bool insyx = false;
        Byte message[4096];
        const MIDIPacket* packet;
        AudioUnit synthUnit;

        AUGraphGetNodeInfo (m_graph, m_synthNode, NULL, NULL, NULL, &synthUnit);
        packet = packets->packet;
        for (i = 0; i < packets->numPackets; ++i) {
            size = 0;
            insyx = ( packet->data[0] == MIDI_STATUS_SYSEX );
            for ( j = 0; j < packet->length; ++j ) {
                if ( packet->data[j] > MIDI_STATUS_ENDSYSEX ) continue;
                if ( (packet->data[j] & MIDI_STATUS_MASK) != 0 &&
                     size > 0 && !insyx ) {
                    midiEvent ( synthUnit, message, size );
                    size = 0;
                }
                message[size++] = packet->data[j];
                insyx &= ( packet->data[j] != MIDI_STATUS_ENDSYSEX );
            }
            if (size > 0)
                midiEvent( synthUnit, message, size );
            packet = MIDIPacketNext (packet);
        }
    }

    void DLSSoftSynth::terminate()
    {
        OSStatus result;
        if (m_graph != 0) {
            result = AUGraphStop(m_graph);
            if (result != noErr)
                kDebug() << "AUGraphStop() err:" << result;
            result = AUGraphClose(m_graph);
            if (result != noErr)
                kDebug() << "AUGraphClose() err:" << result;
            result = DisposeAUGraph(m_graph);
            if (result != noErr)
                kDebug() << "DisposeAUGraph() err:" << result;
        }
        m_graph = 0;
        //m_settings->setOutput_connection(QString());
    }

    void DLSSoftSynth::start()
    {
        OSStatus result;
        ComponentDescription cd;
        AudioUnit synthUnit;
        FSRef fsRef;
        UInt32 usesReverb;
        QString fileName;

        m_messages.clear();

        cd.componentManufacturer = kAudioUnitManufacturer_Apple;
        cd.componentFlags = 0;
        cd.componentFlagsMask = 0;

        result = NewAUGraph (&m_graph);
        if (result != noErr) {
            registerStatus("NewAUGraph", result);
            emit synthErrors(prettyName(), m_messages);
            return;
        }

        cd.componentType = kAudioUnitType_MusicDevice;
        cd.componentSubType = kAudioUnitSubType_DLSSynth;
        result = AUGraphAddNode (m_graph, &cd, &m_synthNode);
        registerStatus( "AUGraphAddNode", result);

        cd.componentType = kAudioUnitType_Output;
        cd.componentSubType = kAudioUnitSubType_DefaultOutput;
        result = AUGraphAddNode (m_graph, &cd, &m_outputNode);
        registerStatus( "AUGraphAddNode", result);

        result = AUGraphOpen (m_graph);
        registerStatus( "AUGraphOpen", result);

        result = AUGraphConnectNodeInput (m_graph, m_synthNode, 0, m_outputNode, 0);
        registerStatus( "AUGraphConnectNodeInput", result);

        result = AUGraphNodeInfo (m_graph, m_synthNode, 0, &synthUnit);
        registerStatus( "AUGraphNodeInfo", result);

        if (!m_settings->default_dls() && !m_settings->soundfont_dls().isEmpty()) {
            fileName = m_settings->soundfont_dls().toLocalFile();
            result = FSPathMakeRef ( (const UInt8*) fileName.toUtf8().data(),
                                     &fsRef, 0);
            registerStatus( "FSPathMakeRef(" + fileName + ')', result);
            result = AudioUnitSetProperty ( synthUnit,
                kMusicDeviceProperty_SoundBankFSRef, kAudioUnitScope_Global,
                0, &fsRef, sizeof (fsRef) );
            registerStatus( "AudioUnitSetProperty(SoundBankFSSpec)", result);
        }

        usesReverb = (m_settings->reverb_dls() ? 1 : 0);
        result = AudioUnitSetProperty ( synthUnit,
            kMusicDeviceProperty_UsesInternalReverb, kAudioUnitScope_Global,
            0, &usesReverb, sizeof (usesReverb) );
        registerStatus( "AudioUnitSetProperty(UsesInternalReverb)", result);

        for (int i = 0; i < 16; ++i) {
            MusicDeviceMIDIEvent(synthUnit, MIDI_STATUS_CONTROLCHANGE+i, 0,0,0);
            MusicDeviceMIDIEvent(synthUnit, MIDI_STATUS_PROGRAMCHANGE+i, 0,0,0);
        }

        result = AUGraphInitialize (m_graph);
        if (result != noErr) {
            registerStatus( "AUGraphInitialize", result);
            emit synthErrors(prettyName(), m_messages);
            return;
        }

        result = AUGraphStart (m_graph);
        if (result != noErr) {
            registerStatus( "AUGraphStart", result);
            emit synthErrors(prettyName(), m_messages);
            return;
        }

        m_settings->setOutput_connection(QLatin1String(PRETTY_NAME));

        emit synthReady(prettyName(), m_messages);
    }

    bool DLSSoftSynth::settingsChanged()
    {
        foreach( const QString& propName, m_settingsNames ) {
            KConfigSkeletonItem* itm = m_settings->findItem(propName);
            if (itm != NULL && !itm->isEqual(m_oldSettingValues[propName])) {
                return true;
            }
        }
        return false;
    }

    void DLSSoftSynth::saveSettingValues()
    {
        foreach( const QString& propName, m_settingsNames ) {
            KConfigSkeletonItem* itm = m_settings->findItem(propName);
            if (itm != NULL)
                m_oldSettingValues[propName] = itm->property();
        }
    }

    QString DLSSoftSynth::prettyName() const
    {
        return QLatin1String(PRETTY_NAME);
    }

}

#include "dlssoftsynth.moc"
