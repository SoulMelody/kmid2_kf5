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

#ifndef MIDIOBJECT_H
#define MIDIOBJECT_H

#include "kmidmacros.h"

#include <QObject>
#include <QUrl>
#include <QVariant>

namespace KMid {

    /**
     * The state the media producing object is in at the moment.
     *
     * \see MIDIObject
     */
    enum State
    {
        /**
         * After construction it might take a while before the Player is
         * ready to play(). Normally this doesn't happen for local
         * files, but can happen for remote files where the asynchronous
         * mimetype detection and prebuffering can take a while.
         */
        LoadingState,
        /**
         * The Player has a valid media file loaded and is ready for
         * playing.
         */
        StoppedState,
        /**
         * The Player is playing a media file.
         */
        PlayingState,
        /**
         * The Player is waiting for data to be able to continue
         * playing.
         */
        BufferingState,
        /**
         * The Player is currently paused.
         */
        PausedState,
        /**
         * An unrecoverable error occurred. The Object is unusable in this state.
         */
        ErrorState
    };

    /**
     * A MIDI Sequencer object
     */
    class KMIDBACKEND_EXPORT MIDIObject : public QObject
    {
        Q_OBJECT
    public:
        MIDIObject(QObject *parent = 0) : QObject(parent) {}
        virtual ~MIDIObject() {}

        /**
         * Return the time interval between two ticks.
         *
         * \returns Returns the tick interval that it was set to (might not
         * be the same as you asked for).
         */
        virtual qint32 tickInterval() const = 0;

        /**
         * Get the current time of the file currently being played.
         */
        virtual qint64 currentTime() const = 0;

        /**
         * Get the current state.
         */
        virtual State state() const = 0;

        /**
         * A translated string describing the error.
         */
        virtual QString errorString() const = 0;

        /**
         * Returns the total time of the media.
         *
         * If the total time is not know return -1. Do not block until it is
         * known, instead emit the totalTimeChanged signal as soon as the total
         * time is known or changes.
         */
        virtual qint64 totalTime() const = 0;

        /**
         * Returns the total time of the media in seconds.
         *
         * If the total time is not know return -1.
         */
        virtual qreal duration() const = 0;

        /**
         * Get the remaining time of the file currently being played.
         *
         * \return The remaining time.
         */
        virtual qint64 remainingTime() const = 0;

        /**
         * Returns the strings associated with the given \p key.
         * The SMF metadata types are documented in the SMF standard
         * specification: http://www.midi.org/techspecs/smf.php
         * 1. Text
         * 2. Copyright
         * 3. Track Name
         * 4. Instrument Name
         * 5. Lyric
         * 6. Marker
         * 7. Cue
         *
         * Backends may also return other Karaoke types as well:
         * KAR_FILETYPE, KAR_VERSION, KAR_INFORMATION,
         * KAR_LANGUAGE, KAR_TITLES, KAR_WARNINGS.
         *
         * A typical usage looks like this:
         * \code
         * setText       (media->metaData("SMF_TEXT"))
         * setCopyright  (media->metaData("SMF_COPYRIGHT"));
         * setTrackNames (media->metaData("SMF_TRACKNAMES"));
         * setInstruments(media->metaData("SMF_INSTRUMENTNAMES"));
         * setLyrics     (media->metaData("SMF_LYRICS"));
         * setMarkers    (media->metaData("SMF_MARKERS"));
         * setCues       (media->metaData("SMF_CUES"));
         * \endcode
         *
         * @param key the metadata key string
         */
        virtual QStringList metaData(const QString &key) const = 0;

        /**
         * Returns the current media source.
         *
         * \see setCurrentSource
         */
        virtual QString currentSource() const = 0;

        /**
         * Set the media source the MediaObject should use.
         *
         * \param source The MediaSource object to the media data. You can
         * just as well use a QUrl or QString (for a local file) here.
         * Setting an empty (invalid) source, will stop and remove the
         * current source.
         *
         * \code
         * QUrl url("http://www.example.com/music.ogg");
         * media->setCurrentSource(url);
         * \endcode
         *
         * \see currentSource
         */
        virtual void setCurrentSource(const QString &source) = 0;

        /**
         * Returns the queued media sources. This does list does not include
         * the current source (returned by currentSource).
         */
        virtual QStringList queue() const = 0;

        /**
         * Set the MediaSources to play when the current media has finished.
         *
         * This function will overwrite the current queue.
         *
         * \see clearQueue
         * \see enqueue
         */
        virtual void setQueue(const QStringList &sources) = 0;

        /**
         * Set the MediaSources to play when the current media has finished.
         *
         * This function overwrites the current queue.
         *
         * \see clearQueue
         * \see enqueue
         */
        virtual void setQueue(const QList<QUrl> &urls) = 0;

        /**
         * Appends one source to the queue. Use this function to provide
         * the next source just in time after the aboutToFinish signal was
         * emitted.
         *
         * \see aboutToFinish
         * \see setQueue
         * \see clearQueue
         */
        virtual void enqueue(const QString &source) = 0;

        /**
         * Appends multiple sources to the queue.
         *
         * \see setQueue
         * \see clearQueue
         */
        virtual void enqueue(const QStringList &sources) = 0;

        /**
         * Appends multiple sources to the queue.
         *
         * \see setQueue
         * \see clearQueue
         */
        virtual void enqueue(const QList<QUrl> &urls) = 0;

        /**
         * Clears the queue of sources.
         */
        virtual void clearQueue() = 0;

        /**
         * Returns the time skew factor (or time speed wrt the real clock time).
         * A factor = 1 means equal to the real clock time. 2 means twice fast,
         * and 0.5 means half time speed.
         *
         * @return The skew factor
         */
        virtual qreal timeSkew() = 0;

        /**
         * Returns the name of the text codec used for lyrics and other texts.
         * @return The text encoding name
         */
        virtual QString getTextEncoding() const = 0;

        /**
         * Returns the song lyrics from the given time.
         * @param time the initial musical time for the text
         */
        virtual QStringList getLyrics(qint64 time) const = 0;

        /**
         * Returns the current song tempo (in qpm).
         */
        virtual qreal currentTempo() = 0;

        /**
         * Returns whether the given MIDI channel is used by the current song.
         * @param channel the MIDI channel
         */
        virtual bool channelUsed(int channel) = 0;

        /**
         * Returns the lowest MIDI note number used by the current song, across
         * all MIDI channels.
         */
        virtual int lowestMidiNote() = 0;

        /**
         * Returns the highest MIDI note number used by the current song,
         * across all MIDI channels.
         */
        virtual int highestMidiNote() = 0;

        /**
         * Returns a text label for the given MIDI channel
         * @param channel the MIDI channel
         */
        virtual QString channelLabel(int channel) = 0;

        /**
         * Guess the text encoding for lyrics and other texts.
         * @return true if a suitable codec has been found
         */
        virtual bool guessTextEncoding() = 0;

        /**
         * Returns the specified song property. The song or the backend
         * implementation may return an empty (invalid) QVariant instance.
         * Some common property keys: SMF_FORMAT, SMF_TRACKS, SMF_DIVISION,
         * NUM_BARS ...
         *
         * @param key the song property key string
         */
        virtual QVariant songProperty(const QString& key) = 0;

        /**
         * Returns the specified property for the given MIDI channel.
         * The song or the backend implementation may return an empty (invalid)
         * QVariant instance. Some common property keys: INITIAL_PATCH, LABEL,
         * USED ...
         *
         * @param channel the MIDI channel
         * @param key the channel property key string
         */
        virtual QVariant channelProperty(int channel, const QString& key) = 0;

    public Q_SLOTS:

        /**
         * Change the interval the tick signal is emitted. If you set \p
         * interval to 0 the signal gets disabled.
         *
         * \param interval tick interval
         *
         * \returns Returns the tick interval that it was set to (might not
         *          be the same as you asked for).
         */
        virtual void setTickInterval(qint32 interval) = 0;

        /**
         * Requests playback of the media data to start. Playback only
         * starts when stateChanged() signals that it goes into PlayingState,
         * though.
         *
         * \par Possible states right after this call:
         * \li BufferingState
         * \li PlayingState
         * \li ErrorState
         */
        virtual void play() = 0;

        /**
         * Requests playback to pause. If it was paused before nothing changes.
         *
         * \par Possible states right after this call:
         * \li PlayingState
         * \li PausedState
         * \li ErrorState
         */
        virtual void pause() = 0;

        /**
         * Requests playback to stop. If it was stopped before nothing changes.
         *
         * \par Possible states right after this call:
         * \li the state it was in before (e.g. PlayingState)
         * \li StoppedState
         * \li ErrorState
         */
        virtual void stop() = 0;

        /**
         * Requests a seek to the time indicated.
         *
         * You can only seek if state() == PlayingState, BufferingState or
         * PausedState.
         *
         * The call is asynchronous, so currentTime can still be the old
         * value right after this method was called. If all you need is a
         * slider that shows the current position and allows the user to
         * seek use the class SeekSlider.
         *
         * @param time The time where to continue playing.
         *
         * \par Possible states right after this call:
         * \li BufferingState
         * \li PlayingState
         * \li ErrorState
         *
         * \see SeekSlider
         */
        virtual void seek(qint64 time) = 0;

        /**
         * Stops and removes all playing and enqueued media sources.
         *
         * \see setCurrentSource
         */
        virtual void clear() = 0;

        /**
         * Sets the time skew factor (or time speed wrt the real clock time).
         * A factor = 1 means equal to the real clock time. 2 means twice fast,
         * and 0.5 means half time speed.
         */
        virtual void setTimeSkew(qreal skew) = 0;

        /**
         * Sets the name of the codec used for lyrics and other texts.
         * @param encoding Text codec name
         */
        virtual void setTextEncoding(const QString& encoding) = 0;

    Q_SIGNALS:
        /**
         * Emitted when the state of the MediaObject has changed.
         * In case you're not interested in the old state you can also
         * connect to a slot that only has one State argument.
         *
         * @param newstate The state the Player is in now.
         * @param oldstate The state the Player was in before.
         */
        void stateChanged(State newstate, State oldstate);

        /**
         * This signal gets emitted every tickInterval.
         *
         * @param time The position of the media file.
         *
         * @see setTickInterval, tickInterval
         */
        void tick(qint64 time);

        /**
         * This signal is emitted whenever the audio/video data that is
         * being played is associated with new meta data. E.g. for radio
         * streams this happens when the next song is played.
         *
         * You can get the new meta data with the metaData methods.
         */
        void metaDataChanged();

        /**
         * Emitted whenever the return value of isSeekable() changes.
         *
         * Normally you'll check isSeekable() first and then let this signal
         * tell you whether seeking is possible now or not. That way you
         * don't have to poll isSeekable().
         *
         * \param isSeekable \p true  if the stream is seekable (i.e. calling
         *                            seek() works)
         *                   \p false if the stream is not seekable (i.e.
         *                            all calls to seek() will be ignored)
         */
        void seekableChanged(bool isSeekable);

        /**
         * Emitted when the object has finished playback.
         * It is not emitted if you call stop(), pause() or
         * load(), but only on end-of-queue or a critical error.
         *
         * \warning This signal is not emitted when the current source has
         * finished and there's another source in the queue. It is only
         * emitted when the queue is empty.
         *
         * \see currentSourceChanged
         * \see aboutToFinish
         * \see prefinishMarkReached
         */
        void finished();

        /**
         * Emitted when the MediaObject makes a transition to the next
         * MediaSource in the queue().
         *
         * In other words, it is emitted when an individual MediaSource is
         * finished.
         *
         * \param newSource The source that starts to play at the time the
         * signal is emitted.
         */
        void currentSourceChanged(const QString &newSource);

        /**
         * Sequenced SMF events (for feedback to the application)
         */
        void tempoChanged(const qreal tempo);
        void timeSignatureChanged(const int numerator, const int denominator);
        void midiText(const int type, const QString &txt);
        void midiNoteOn(const int chan, const int note, const int vel);
        void midiNoteOff(const int chan, const int note, const int vel);
        void midiController(const int chan, const int control, const int value);
        void midiKeyPressure(const int chan, const int note, const int value);
        void midiProgram(const int chan, const int program);
        void midiChannelPressure(const int chan, const int value);
        void midiPitchBend(const int chan, const int value);
        void midiSysex(const QByteArray &data);
        void beat(const int bar, const int beat, const int max);

    };

}

#endif /* MIDIOBJECT_H */
