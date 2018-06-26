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

#ifndef KMID2_H
#define KMID2_H

#include "backend.h"
#include "midimapper.h"
#include "ui_prefs_lyrics.h"
#include "ui_prefs_midi.h"

#include <KXmlGuiWindow>
#include <KProcess>
#include <QDBusVariant>

class Pianola;
class Channels;
class RhythmView;
class TimeLabel;
class KToggleAction;
class KAction;
class QUrl;
class KComboBox;
class KTextEdit;
class KRecentFilesAction;

namespace KMid {
      class Backend;
      class MIDIObject;
      class MIDIOutput;
      class BackendLoader;
}

using namespace KMid;

/**
 * This class serves as the main window for KMid2.  It handles the
 * menus, toolbars, and status bars.
 *
 * @short Main window class
 * @author Pedro Lopez-Cabanillas <plcl@users.sf.net>
 * @version 0.1
 */
class KMid2 : public KXmlGuiWindow
{
    Q_OBJECT
    Q_PROPERTY(bool autoStart READ autoStart WRITE setAutoStart)
    Q_PROPERTY(QString midiConnection READ midiConnection WRITE setMidiConnection)
    Q_PROPERTY(double tempoFactor READ tempoFactor WRITE setTempoFactor)
    Q_PROPERTY(double volumeFactor READ volumeFactor WRITE setVolumeFactor)
    Q_PROPERTY(int transpose READ transpose WRITE setTranspose)
    Q_PROPERTY(qlonglong length READ length)
    Q_PROPERTY(bool looping READ isLooping WRITE setLooping)
    Q_PROPERTY(qlonglong position READ position)
    Q_PROPERTY(int state READ state)

public:
    /**
     * Default Constructor
     */
    KMid2();

    /**
     * Default Destructor
     */
    virtual ~KMid2();

    void setPlayList(const QList<QUrl> &urls);
    void setUrlsLater(const QList<QUrl> &urls);
    void dragEnterEvent(QDragEnterEvent* event);
    void dropEvent(QDropEvent* event);
    void showEvent(QShowEvent* event);
    void reload();
    bool isMuted(int channel);
    QStringList metaData(const QString& key);
    bool autoStart();
    QString midiConnection();
    double tempoFactor();
    int transpose();
    bool isLooping();
    qlonglong length();
    qlonglong position();
    int state();
    double volumeFactor();
    QString currentSource() const;
    bool openUrl(const QString& url);
    QDBusVariant songProperty(const QString& key);
    QDBusVariant channelProperty(int channel, const QString& key);

public slots:
    void setMuted(int channel, bool muted);
    void stop();
    void play();
    void pause();
    void setAutoStart(bool enable);
    void setLooping(bool enable);
    void setMidiConnection(const QString& conn);
    void setTempoFactor(double factor);
    void setTranspose(int amount);
    void setVolumeFactor(double factor);
    void seek(qlonglong pos);

private slots:
    void fileOpen();
    void fileSaveLyrics();
    void filePrint();
    void rewind();
    void forward();
    void previous();
    void next();
    void finished();
    void slotLoaded(Backend *backend, const QString& library, const QString& name);
    void slotUpdateState(State newState, State oldState);
    void slotSelectEncoding(int i);
    void slotSelectEncoding(const QString& encoding);
    void slotMidiTextEvent(const int type, const QString &txt);
    void slotBeat(const int bars, const int beats, const int maxbeats);
    void slotTimeSignatureEvent(const int numerator, const int denominator);
    void slotTempoReset();
    void slotTempoSlider(int value);
    void slotTimeSlider(int value);
    void slotVolumeSlider(int value);
    void slotPitchSlider(int value);
    void slotTimeSliderPressed();
    void slotTempoSliderMoved(int value);
    void slotVolumeSliderMoved(int value);
    void slotPitchSliderMoved(int value);
    void slotTick(qint64 tick);
    void slotEditSettings();
    void slotSourceChanged(const QString &src);
    void slotModeChanged(int mode);
    void slotFileInfo();
    void slotReadSettings();
    void slotWriteSettings();
    void slotApplySettings(const QString&);
    void slotSavePlaylist();
    void slotLoadPlaylist();
    void slotManagePlaylist();
    void slotShowPianola(bool checked);
    void slotShowChannels(bool checked);
    void slotPianolaClosed();
    void slotChannelsClosed();
    void slotShuffle(bool checked);
    void slotURLSelected(const QUrl& url);
    void slotUpdateConfigDialogWidgets();
    void slotCheckOutput();
    void slotSaveSongSettings();
    void slotLoadSongSettings();
    void slotSoftSynthStarted(const QString& pgm, const QStringList& messages);
    void slotSoftSynthErrors(const QString& pgm, const QStringList& messages);
    void slotBackendChanged(int index);
    void slotDockVolLocationChanged ( Qt::DockWidgetArea area );
    void slotTempoChanged(qreal);

signals:
    void playerStateChanged(int state);
    void tempoEvent(double tempo);
    void timeSignatureEvent(int num, int den);
    void midiTextEvent(int type, const QString& text);
    void midiNoteOnEvent(int channel, int note, int velocity);
    void midiNoteOffEvent(int channel, int note, int velocity);
    void midiControllerEvent(int channel, int ctl, int value);
    void midiKeyPressureEvent(int channel, int note, int value);
    void midiProgramEvent(int channel, int program);
    void midiChannelPressureEvent(int channel, int value);
    void midiPitchBendEvent(int channel, int value);
    void beat(int bar, int beat, int max);
    void tick(qlonglong time);
    void playerFinished();
    void sourceChanged(const QString& source);

private:
    void setupDockWidgets();
    void setupActions();
    void setupPlaces();
    void updateState(const QString &newState, const QString &stateName);
    void initialize();
    void displayLyrics();
    void updateTempoLabel();
    bool queryExit();
    void displayBeat(const int bars, const int beats);
    void connectMidiOutput();
    void loadPlaylist(const QString &fileName);
    void readProperties(const KConfigGroup &cfg);
    void saveProperties(KConfigGroup &cfg);
    QString chooseInitialConnection(const QStringList& items);

private:
    bool m_connected;
    bool m_seeking;
    qint64 m_seekamt;
    qreal m_rtempo;
    BackendLoader *m_loader;
    Backend *m_currentBackend;
    MIDIObject *m_midiobj;
    MIDIOutput *m_midiout;

    KToggleAction *m_toolbarAction;
    KToggleAction *m_statusbarAction;
    KToggleAction *m_pause;
    KToggleAction *m_loop;
    KToggleAction *m_autostart;
    KToggleAction *m_playListAuto;
    KToggleAction *m_showPianola;
    KToggleAction *m_showChannels;
    KToggleAction *m_shuffle;

    KAction *m_play;
    KAction *m_forward;
    KAction *m_rewind;
    KAction *m_stop;
    KAction *m_previous;
    KAction *m_next;
    KAction *m_fileInfo;
    KAction *m_fileSaveLyrics;
    KAction *m_playListSave;
    KAction *m_playListLoad;
    KAction *m_playListEdit;
    KAction *m_print;
    KAction *m_saveSongSettings;
    KAction *m_loadSongSettings;
    KToggleAction *m_autoSongSettings;
    KRecentFilesAction *m_recentFiles;

    QPointer<Pianola> m_pianola;
    QPointer<Channels> m_channels;
    QString m_songName;
    QString m_songEncoding;
    QString m_playList;

    MidiMapper m_mapper;
    QByteArray m_resetMessage;
    Ui::prefs_midi ui_prefs_midi;
    Ui::prefs_lyrics ui_prefs_lyrics;

    QSlider *m_volumeSlider;
    QSlider *m_pitchSlider;
    QSlider *m_timeSlider;
    QSlider *m_tempoSlider;
    QLabel *m_labelTempo;
    QLabel *m_labelBeatCount;
    TimeLabel *m_timeLabel;
    QPushButton *m_btnResetTempo;
    RhythmView *m_rhythm;
    KComboBox *m_comboCodecs;
    KTextEdit *m_lyricsText;
    QList<QUrl> m_pendingList;

    struct MidiBackend {
        QString  library;
        QString  name;
        Backend* backend;
    };
    QList<MidiBackend> m_backends;
    QString m_currentBackendLibrary;

    Settings *m_settings;
    QDockWidget *m_volDock;
    QDockWidget *m_posDock;
    QDockWidget *m_rthmDock;
    QDockWidget *m_encDock;
};

#endif // KMID2_H
