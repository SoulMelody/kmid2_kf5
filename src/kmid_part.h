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

#ifndef KMID_PART_H
#define KMID_PART_H

#include <KF5/KMediaPlayer/kmediaplayer/player.h>
#include <QDBusVariant>

#ifndef KMIDPART_EXPORT
# if defined(kmid_part_EXPORTS) || defined(MAKE_KMID_PART_LIB)
#  define KMIDPART_EXPORT KDE_EXPORT
# else
#  define KMIDPART_EXPORT KDE_IMPORT
# endif
#endif

class K4AboutData;

namespace KMid {
    class Backend;
};
using namespace KMid;

/**
 * KMidPart is an implementation of the KMediaPlayer::Player interface for
 * the MIDI/Karaoke player KPart component.
 *
 * This KPart provides a minimal user interface and the basic functionality of
 * the KMid player, for including in other KDE programs.
 *
 * Note: the time unit is ticks (musical time) instead of milliseconds, in all
 * methods having a time argument, even in the methods inherited from the
 * KMediaPlayer interfaces.
 *
 * @short MIDI/Karaoke player KPart component
 * @author Pedro Lopez-Cabanillas <plcl@users.sf.net>
 * @version 0.1
 * @see http://api.kde.org/4.x-api/kdelibs-apidocs/interfaces/kmediaplayer/
 */
class KMIDPART_EXPORT KMidPart : public KMediaPlayer::Player
{
    Q_OBJECT
    Q_PROPERTY(bool autoStart READ autoStart WRITE setAutoStart)
    Q_PROPERTY(QString midiConnection READ midiConnection WRITE setMidiConnection)
    Q_PROPERTY(double tempoFactor READ tempoFactor WRITE setTempoFactor)
    Q_PROPERTY(double volumeFactor READ volumeFactor WRITE setVolumeFactor)
    Q_PROPERTY(int transpose READ transpose WRITE setTranspose)

public:
    /**
     * Constructor for the KMediaPlayer/Engine component
     */
    KMidPart(QObject *parent);

    /**
     * Constructor for the KMediaPlayer/Player component
     */
    KMidPart(QWidget *parentWidget,QObject *parent, const QVariantList&);

    /**
     * Destructor
     */
    virtual ~KMidPart();

    /**
     * Returns the View component
     *
     * @return a KMediaPlayer::View pointer or 0 if there is no view.
     * @see KMediaPlayer::Player::view()
     */
    virtual KMediaPlayer::View* view ();

    /**
     * Returns whether the current track honors seek requests.
     * Always true for MIDI files.
     *
     * @return true when a MIDI/karaoke file is loaded.
     * @see KMediaPlayer::Player::isSeekable()
     */
    virtual bool isSeekable (void) const;

    /**
     * Returns the current playback position in the track. Like the other
     * methods in the component, the time unit is ticks (musical time).
     *
     * @return the time position in ticks
     * @see KMediaPlayer::Player::position()
     */
    virtual qlonglong position (void) const;

    /**
     * Returns whether the current track has a length.
     * Always true for MIDI files.
     *
     * @return true when a MIDI/karaoke file is loaded.
     * @see KMediaPlayer::Player::hasLength()
     */
    virtual bool hasLength (void) const;

    /**
     * @see KMediaPlayer::Player::length()
     */
    virtual qlonglong length (void) const;

    /**
     * Like the other methods in the component, the time unit is ticks (musical time).
     *
     * @param ticks time in ticks
     * @see KMediaPlayer::Player::seek()
     */
    virtual void seek (qlonglong ticks);

    /**
     * Returns a KAboutData instance pointer for this component.
     */
    static K4AboutData *createAboutData();

    /**
     * Returns if the tracks should automatically start playing once loaded.
     */
    bool autoStart();

    /**
     * Returns the MIDI port name connected to the MIDI OUT of this component
     */
    QString midiConnection();

    /**
     * Returns the meta-data extracted from the loaded MIDI file.
     *
     * The key argument may be one of the following strings:
     * SMF_TEXT (arbitrary texts)
     * SMF_COPYRIGHT (copyright strings)
     * SMF_TRACKNAMES (track names)
     * SMF_INSTRUMENTNAMES (instrument names)
     * SMF_LYRICS (lyrics)
     * SMF_MARKERS (markers)
     * SMF_CUES (cue points)
     *
     * If the track is a Karaoke file (usually with ".kar" extension) there may
     * be more keys available: KAR_FILETYPE, KAR_VERSION, KAR_INFORMATION,
     * KAR_LANGUAGE, KAR_TITLES, and KAR_WARNINGS.
     */
    QStringList metaData(const QString& key);

    /**
     * Returns the tempo (speed) factor between 0.5 and 2.0
     */
    double tempoFactor();

    /**
     * Returns the volume factor, between 0.0 and 2.0
     */
    double volumeFactor();

    /**
     * Returns the transpose amount in semitones, between -12 and +12
     */
    int transpose();

    /**
     * Returns the muted state for the specified MIDI channel
     * @param channel a MIDI channel (0 to 15)
     */
    bool isMuted(int channel);

    /**
     * Returns the name of the currently loaded sequence, if any.
     */
    QString currentSource() const;

    /**
     * Returns the specified song property. The song or the backend
     * implementation may return an empty (invalid) QVariant instance.
     * Some common property keys: SMF_FORMAT, SMF_TRACKS, SMF_DIVISION,
     * NUM_BARS ...
     *
     * @param key the song property key string
     */
    QDBusVariant songProperty(const QString& key);

    /**
     * Returns the specified property for the given MIDI channel.
     * The song or the backend implementation may return an empty (invalid)
     * QVariant instance. Some common property keys: INITIAL_PATCH, LABEL,
     * USED ...
     *
     * @param channel the MIDI channel
     * @param key the channel property key string
     */
    QDBusVariant channelProperty(int channel, const QString& key);

public slots:
    /**
     * Pauses the playback
     */
    virtual void pause (void);

    /**
     * Starts the playback
     */
    virtual void play (void);

    /**
     * Stops the playback
     */
    virtual void stop (void);

    /**
     * Sets the auto-start feature (enabled by default)
     *
     * @param start true to enable or false to disable the auto-start
     */
    void setAutoStart(bool start);

    /**
     * Connects the MIDI OUT port to the specified port name
     *
     * @param conn MIDI port name for the MIDI OUT connection
     */
    void setMidiConnection(const QString conn);

    /**
     * Sets the tempo (speed) factor
     *
     * @param f tempo (speed) factor between 0.5 and 2.0
     */
    void setTempoFactor(double f);

    /**
     * Sets the volume factor
     *
     * @param f volume factor between 0.0 and 2.0
     */
    void setVolumeFactor(double f);

    /**
     * Sets the transpose amount in semitones
     *
     * @param t transpose amount in semitones, between -12 and +12
     */
    void setTranspose(int t);

    /**
     * Sets the mute state for the specified MIDI channel
     *
     * @param channel MIDI channel, between 0 and 15
     * @param muted state
     */
    void setMuted(int channel, bool muted);

    /**
     * Reloads the current track, preserving the time position in ticks
     */
    void reload();

    virtual bool openUrl(const QUrl&);
    virtual bool openUrl(const QString&);

signals:
    /**
     * Emitted when a tempo change is played
     *
     * @param changed tempo, including the applied tempo factor
     */
    void tempoEvent(double tempo);

    /**
     * Emitted when a time signature (rhythm specification) is played
     *
     * @param num rhythm numerator
     * @param den rhythm denominator
     */
    void timeSignatureEvent(int num, int den);

    /**
     * Emitted when a text event (lyric or otherwise) is played
     *
     * @param type SMF metadata type (1=text, 5=lyric, ...)
     * @param text event data
     */
    void midiTextEvent(int type, const QString& text);

    /**
     * Emitted when a MIDI note on event is played
     *
     * @param channel MIDI channel, between 0 and 15
     *
     */
    void midiNoteOnEvent(int channel, int note, int velocity);

    /**
     * Emitted when a MIDI note off event is played
     *
     * @param channel MIDI channel, between 0 and 15
     * @param note MIDI note
     * @param velocity MIDI velocity
     */
    void midiNoteOffEvent(int channel, int note, int velocity);

    /**
     * Emitted when a MIDI control change event is played
     *
     * @param channel MIDI channel, between 0 and 15
     * @param ctl MIDI controller
     * @param value controller value
     */
    void midiControllerEvent(int channel, int ctl, int value);

    /**
     * Emitted when a MIDI polyphonic pressure event is played
     *
     * @param channel MIDI channel, between 0 and 15
     * @param note MIDI note
     * @param value pressure value
     */
    void midiKeyPressureEvent(int channel, int note, int value);

    /**
     * Emitted when a MIDI program event is played
     *
     * @param channel MIDI channel, between 0 and 15
     * @param value program value
     */
    void midiProgramEvent(int channel, int program);

    /**
     * Emitted when a MIDI channel pressure event is played
     *
     * @param channel MIDI channel, between 0 and 15
     * @param value pressure value
     */
    void midiChannelPressureEvent(int channel, int value);

    /**
     * Emitted when a MIDI pitch bend event is played
     *
     * @param channel MIDI channel, between 0 and 15
     * @param value pitch bender value
     */
    void midiPitchBendEvent(int channel, int value);

    /**
     * Emitted when each beat gets played
     *
     * @param bar measure (bar) number
     * @param beat beat number
     * @param max number of beats for the current measure
     */
    void beat(int bar, int beat, int max);

    /**
     * This signal gets emitted every tickInterval.
     *
     * @param time The position of the media file in ticks.
     */
    void tick(qlonglong time);

    /**
     * Emitted when the track has finished playback.
     */
    void finished();

    /**
     * Emitted when the player loads a new track
     *
     * @param source the new loaded track
     */
    void sourceChanged(const QString& source);

private slots:
    void slotLoaded(Backend *backend, const QString& library, const QString& name);
    void slotUpdateState(State, State);
    void slotSoftSynthStarted(const QString& pgm, const QStringList& messages);
    void slotSoftSynthErrors(const QString& pgm, const QStringList& messages);
    void slotSeek(int value);
    void slotTick(qint64);
    void slotFinished();
    void slotSourceChanged(QString);
    void slotTempoChanged(qreal);

protected:
    virtual bool openFile();

private:
    void setupActions();
    void initialize();
    void connectMidiOutput();

    class KMidPartPrivate;
    KMidPartPrivate *d;
};

#endif // KMID_PART_H
