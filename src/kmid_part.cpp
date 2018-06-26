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

#include "kmid_part.h"
#include "kmid_partview.h"
#include "kmidpartadaptor.h"
#include "backend.h"
#include "backendloader.h"
#include "settings.h"
#include "midimapper.h"

#include "KF5/KDELibs4Support/kaction.h"
#include <KF5/KWidgetsAddons/KToggleAction>
#include <KActionCollection>
#include "KF5/KDELibs4Support/kcomponentdata.h"
#include "KF5/KDELibs4Support/k4aboutdata.h"
#include <KFileDialog>
#include <KStandardAction>
#include <KMessageBox>
#include <KMediaPlayer/Player>
#include <KF5/KCoreAddons/KPluginFactory>
#include <KF5/KCoreAddons/KPluginLoader>
#include "KF5/KDELibs4Support/klocale.h"

#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>
#include <QtDBus/QtDBus>
#include <QtWidgets/QTextEdit>

/*
K_PLUGIN_FACTORY( KMidPartFactory, registerPlugin<KMidPart>(); )
K_EXPORT_PLUGIN( KMidPartFactory("kmid_part") )
K_PLUGIN_FACTORY_WITH_JSON(KMidPartFactory,
                           "kmid_part.json",
                           registerPlugin<KMidPart>();)*/

struct MidiBackend {
    QString  library;
    QString  name;
    KMid::Backend* backend;
};

class KMidPart::KMidPartPrivate {
public:
    KMidPartPrivate(KMidPart *parent, QWidget *parentWidget) :
        q(parent),
        m_parentWidget(parentWidget),
        m_view(0),
        m_loader(0),
        m_currentBackend(0),
        m_midiobj(0),
        m_midiout(0),
        m_settings(new Settings),
        m_mapper(new KMid::MidiMapper),
        m_autoStart(true),
        m_volfactor(1.0),
        m_playerReady(false),
        m_playPending(false)
    {
        if (parentWidget != 0)
            m_view = new KMidPartView(parentWidget);
    }

    virtual ~KMidPartPrivate() {
        delete m_mapper;
        delete m_settings;
    }

    KMidPart *q;
    QWidget *m_parentWidget;
    KMidPartView *m_view;
    KMid::BackendLoader *m_loader;
    KMid::Backend *m_currentBackend;
    KMid::MIDIObject *m_midiobj;
    KMid::MIDIOutput *m_midiout;
    Settings *m_settings;
    KMid::MidiMapper *m_mapper;
    KToggleAction *m_pause;
    KAction *m_play;
    KAction *m_stop;
    QByteArray m_resetMessage;
    QList<MidiBackend> m_backends;
    QString m_currentBackendLibrary;
    bool m_autoStart;
    double m_volfactor;
    bool m_playerReady;
    bool m_playPending;
    QMutex m_connmutex;
};

KMidPart::KMidPart( QWidget *parentWidget, QObject *parent, const QVariantList& args )
    : KMediaPlayer::Player(parentWidget, "KMidPlayer", parent)
{
    qDebug() << "KMediaPlayer/Player constructor"
             << "parentWidget:" << parentWidget
             << "parent:" << parent;
    if (parentWidget == 0)
        parentWidget = qobject_cast<QWidget*>(parent);
    d = new KMidPartPrivate(this, parentWidget);
    //setComponentData( KMidPartFactory::componentData() );
    (void) new KMidPartAdaptor(this);
    QDBusConnection::sessionBus().registerObject(QLatin1String("/KMidPart"), this);
    setupActions();
    setXMLFile("kmid_part.rc");
    setWidget(d->m_view);
    initialize();
    foreach (const QVariant& v, args) {
    	QString a = v.toString().toLower();
    	int p = a.indexOf('=');
    	if (p > -1) {
    		QString key = a.left(p);
    		QString val = a.mid(p+1);
    		val = val.remove('"');
    		val = val.remove('\'');
    		if (key == "autostart") {
    			setAutoStart( val == "true" );
    		} else if (key == "loop") {
    			setLooping( val == "true" );
    		} else if (key == "volume") {
    			double v = val.toDouble() / 100.0;
  				setVolumeFactor(v);
    		}
    	}
    }
}

KMidPart::KMidPart( QObject *parent )
    : KMediaPlayer::Player(parent),
      d(new KMidPartPrivate(this, 0))
{
    qDebug() << "KMediaPlayer/Engine constructor"
             << "parent:" << parent;
    //setComponentData( KMidPartFactory::componentData() );
    QDBusConnection::sessionBus().registerObject(QLatin1String("/KMidPart"), this);
    setupActions();
    setXMLFile("kmid_part.rc");
    setWidget(0);
    initialize();
}

KMidPart::~KMidPart()
{
    stop();
    if (d->m_midiout != 0) {
        d->m_midiout->allNotesOff();
        d->m_midiout->resetControllers();
    }
    delete d;
}

void KMidPart::setupActions()
{
    d->m_play = new KAction(this);
    d->m_play->setText(i18nc("@action player play", "Play") );
    d->m_play->setIcon(QIcon("media-playback-start"));
    d->m_play->setShortcut( Qt::Key_MediaPlay );
    d->m_play->setWhatsThis(i18nc("@info:whatsthis","Start playback of the current session"));
    d->m_play->setEnabled(false);
    connect(d->m_play, SIGNAL(triggered()), SLOT(play()));
    actionCollection()->addAction("play", d->m_play);

    d->m_pause = new KToggleAction(this);
    d->m_pause->setText(i18nc("@action player pause", "Pause") );
    d->m_pause->setIcon(QIcon("media-playback-pause"));
    d->m_pause->setWhatsThis(i18nc("@info:whatsthis","Pause the playback"));
    d->m_pause->setEnabled(false);
    connect(d->m_pause, SIGNAL(triggered()), SLOT(pause()));
    actionCollection()->addAction("pause", d->m_pause);

    d->m_stop = new KAction(this);
    d->m_stop->setText(i18nc("@action player stop", "Stop") );
    d->m_stop->setIcon(QIcon("media-playback-stop"));
    d->m_stop->setShortcut( Qt::Key_MediaStop );
    d->m_stop->setWhatsThis(i18nc("@info:whatsthis","Stop playback or recording") );
    d->m_stop->setEnabled(false);
    connect(d->m_stop, SIGNAL(triggered()), SLOT(stop()));
    actionCollection()->addAction("stop", d->m_stop);

    if (d->m_view != 0) {
        connect(d->m_view, SIGNAL(play()), SLOT(play()));
        connect(d->m_view, SIGNAL(pause()), SLOT(pause()));
        connect(d->m_view, SIGNAL(stop()), SLOT(stop()));
        connect(d->m_view, SIGNAL(seek(int)), SLOT(slotSeek(int)));
        connect(d->m_view, SIGNAL(volume(double)), SLOT(setVolumeFactor(double)));
        connect(d->m_view, SIGNAL(transpose(int)), SLOT(setTranspose(int)));
        connect(d->m_view, SIGNAL(speed(double)), SLOT(setTempoFactor(double)));
    }
}

void KMidPart::initialize()
{
    d->m_loader = new KMid::BackendLoader(this);
    connect(d->m_loader, SIGNAL(loaded(Backend*,const QString&,const QString&)),
                      SLOT(slotLoaded(Backend*,const QString&,const QString&)));
    d->m_loader->loadAllBackends();
    if (d->m_currentBackend == 0) {
        KMessageBox::error(d->m_parentWidget, i18nc("@info","No MIDI backend loaded."),
                i18nc("@title:window","Fatal"));
    } else {
        QString mapperFile = d->m_settings->midi_mapper();
        if (mapperFile.isEmpty())
            d->m_mapper->clear();
        else {
            d->m_mapper->loadFile(mapperFile);
            if (d->m_midiout != NULL) d->m_midiout->setMidiMap(d->m_mapper);
        }
        switch(d->m_settings->reset_mode()) {
        case Settings::None:
            d->m_resetMessage.clear();
            break;
        case Settings::GM:
            d->m_resetMessage = QByteArray::fromRawData (
                    reinterpret_cast<const char *>(gmreset), sizeof(gmreset) );
            break;
        case Settings::GS:
            d->m_resetMessage = QByteArray::fromRawData (
                    reinterpret_cast<const char *>(gsreset), sizeof(gsreset) );
            break;
        case Settings::XG:
            d->m_resetMessage = QByteArray::fromRawData (
                    reinterpret_cast<const char *>(xgreset), sizeof(xgreset) );
            break;
        case Settings::Syx:
            QFile file(d->m_settings->sysex_file().toLocalFile());
            file.open(QIODevice::ReadOnly);
            d->m_resetMessage = file.readAll();
            file.close();
            break;
        }
        d->m_midiout->setResetMessage(d->m_resetMessage);
        if (d->m_currentBackend->hasSoftSynths()) {
            connect( d->m_currentBackend,
                SIGNAL(softSynthStarted(const QString&,const QStringList&)),
                SLOT(slotSoftSynthStarted(const QString&,const QStringList&)) );
            connect( d->m_currentBackend,
                SIGNAL(softSynthErrors(const QString&,const QStringList&)),
                SLOT(slotSoftSynthErrors(const QString&,const QStringList&)) );
        }
        if ( d->m_midiout != 0) {
            if (d->m_settings->exec_fluid() || d->m_settings->exec_timidity())
                qDebug() << "waiting for a soft synth";
            else
                connectMidiOutput();
        }
        slotUpdateState(Empty,Empty);
    }
}

void KMidPart::slotLoaded( Backend *backend,
        const QString& library, const QString& name)
{
    MidiBackend midiBackend;
    midiBackend.backend = backend;
    midiBackend.library = library;
    midiBackend.name = name;
    d->m_backends.append(midiBackend);
    backend->setParent(this);
    qDebug() << library << name << backend->initialized();
    if ( backend != NULL && backend->initialized() &&
         d->m_currentBackend == NULL &&
         (d->m_settings->midi_backend().isEmpty() ||
          d->m_settings->midi_backend() == library) ) {
        d->m_midiobj = backend->midiObject();
        d->m_midiout = backend->midiOutput();
        d->m_midiout->setMidiMap(d->m_mapper);

        connect(d->m_midiobj, SIGNAL(stateChanged(State,State)),
                SLOT(slotUpdateState(State,State)));
        connect(d->m_midiobj, SIGNAL(tick(qint64)),
                SLOT(slotTick(qint64)));
        connect(d->m_midiobj, SIGNAL(finished()),
                SLOT(slotFinished()));
        connect(d->m_midiobj, SIGNAL(currentSourceChanged(QString)),
                SLOT(slotSourceChanged(QString)));
        connect(d->m_midiobj, SIGNAL(tempoChanged(qreal)),
                SLOT(slotTempoChanged(qreal)));
        connect(d->m_midiobj, SIGNAL(beat(int,int,int)),
                SIGNAL(beat(int,int,int)));
        connect(d->m_midiobj, SIGNAL(timeSignatureChanged(int,int)),
                SIGNAL(timeSignatureEvent(int,int)));
        connect(d->m_midiobj, SIGNAL(midiText(int,const QString&)),
                SIGNAL(midiTextEvent(int,const QString&)));
        connect(d->m_midiobj, SIGNAL(midiNoteOn(int,int,int)),
                SIGNAL(midiNoteOnEvent(int,int,int)));
        connect(d->m_midiobj, SIGNAL(midiNoteOff(int,int,int)),
                SIGNAL(midiNoteOffEvent(int,int,int)));
        connect(d->m_midiobj, SIGNAL(midiController(int,int,int)),
                SIGNAL(midiControllerEvent(int,int,int)));
        connect(d->m_midiobj, SIGNAL(midiKeyPressure(int,int,int)),
                SIGNAL(midiKeyPressureEvent(int,int,int)));
        connect(d->m_midiobj, SIGNAL(midiProgram(int,int)),
                SIGNAL(midiProgramEvent(int,int)));
        connect(d->m_midiobj, SIGNAL(midiChannelPressure(int,int)),
                SIGNAL(midiChannelPressureEvent(int,int)));
        connect(d->m_midiobj, SIGNAL(midiPitchBend(int,int)),
                SIGNAL(midiPitchBendEvent(int,int)));

        if (backend->hasSoftSynths())
            backend->initializeSoftSynths(d->m_settings);
        d->m_currentBackend = backend;
        d->m_currentBackendLibrary = library;
    }
}

void KMidPart::slotSoftSynthStarted(const QString& pgm, const QStringList& messages)
{
    if (!messages.isEmpty())
        KMessageBox::informationList(d->m_parentWidget,
            i18ncp("@info", "%2 has returned the following message "
                    "when launched with the provided arguments.",
                    "%2 has returned the following messages "
                    "when launched with the provided arguments.",
                    messages.size(),
                    pgm),
            messages,
            i18ncp("@title:window", "%2 message", "%2 messages",
                    messages.size(),
                    pgm),
            "softsynth_warnings");
    connectMidiOutput();
}

void KMidPart::slotSoftSynthErrors(const QString& pgm, const QStringList& messages)
{
    KMessageBox::error(d->m_parentWidget,
        i18ncp("@info", "Failed to run %2 with the provided "
                "arguments.<nl/>Returned message:<nl/>%3",
                "Failed to run %2 with the provided "
                "arguments.<nl/>Returned messages:<nl/>%3",
                messages.size(),
                pgm, messages.join("<nl/>")),
        i18nc("@title:window", "%1 startup failed", pgm));
}

void KMidPart::connectMidiOutput()
{
    QMutexLocker locker(&d->m_connmutex);
    bool success = false;
    QString conn = d->m_settings->output_connection();
    if (conn.isEmpty()) {
        QStringList items = d->m_midiout->outputDeviceList();
        conn = items.first();
        success = d->m_midiout->setOutputDeviceName(conn);
        if (success)
            d->m_settings->setOutput_connection(conn);
    } else {
        success = d->m_midiout->setOutputDeviceName(conn);
    }
    qDebug() << "connection to" << conn << "result:" << success;
    d->m_playerReady = success;
    if (success && d->m_playPending) {
        locker.unlock();
        play();
    }
}

void KMidPart::slotUpdateState(State newState, State /*oldState*/)
{
    KMid::State kmidstate = static_cast<KMid::State>(newState);
    switch(kmidstate) {
    case KMid::PlayingState:
        //qDebug() << "State: play";
        setState(Play);
        //stateChanged("playing_state");
        d->m_play->setEnabled(false);
        d->m_pause->setEnabled(true);
        d->m_pause->setChecked(false);
        d->m_stop->setEnabled(true);
        if (d->m_view != 0)
            d->m_view->setPlayingState(true);
        break;
    case KMid::PausedState:
        //qDebug() << "State: pause";
        setState(Pause);
        break;
    case KMid::StoppedState:
        //qDebug() << "State: stop";
        setState(Stop);
        //stateChanged("stopped_state");
        d->m_play->setEnabled(true);
        d->m_pause->setEnabled(false);
        d->m_pause->setChecked(false);
        d->m_stop->setEnabled(false);
        if (d->m_view != 0)
            d->m_view->setPlayingState(false);
        break;
    default:
        //qDebug() << "State: empty";
        setState(Empty);
        //stateChanged("disabled_state");
        d->m_play->setEnabled(false);
        d->m_pause->setEnabled(false);
        d->m_pause->setChecked(false);
        d->m_stop->setEnabled(false);
        if (d->m_view != 0)
            d->m_view->setPlayingState(false);
        break;
    }
}

void KMidPart::slotTick(qint64 ticks)
{
    if (d->m_view != 0)
        d->m_view->setPosition(ticks);
    emit tick(ticks);
}

void KMidPart::slotFinished()
{
    if (d->m_view != 0)
        d->m_view->setPosition(0);
    if (isLooping())
        play();
    emit finished();
}

K4AboutData *KMidPart::createAboutData()
{
    K4AboutData *aboutData = new K4AboutData( "kmid_part", 0,
            ki18nc("@title", "KMidPart"), "0.1");
    aboutData->addAuthor( ki18nc("@info:credit", "Pedro Lopez-Cabanillas"),
            ki18nc("@info:credit", "Maintainer"), "plcl@users.sf.net");
    return aboutData;
}

bool KMidPart::openFile()
{
    QString localFile = localFilePath();
    if (d->m_midiobj != 0) {
        d->m_midiobj->setCurrentSource(localFile);
        d->m_midiobj->seek(0);
        if (d->m_view != 0)
            d->m_view->resetTimePosition(d->m_midiobj->totalTime());
    }
    return true;
}

KMediaPlayer::View* KMidPart::view ()
{
    return d->m_view;
}

bool KMidPart::isSeekable() const
{
    return true;
}

qlonglong KMidPart::position() const
{
    if (d->m_midiobj != 0)
        return d->m_midiobj->currentTime();
    return 0;
}

bool KMidPart::hasLength() const
{
    return true;
}

qlonglong KMidPart::length() const
{
    if (d->m_midiobj != 0)
        return d->m_midiobj->totalTime();
    return 0;
}

void KMidPart::seek(qlonglong value)
{
    if (d->m_midiobj != 0) {
        d->m_midiobj->seek(value);
        if ((state() != Play) && (d->m_view != 0))
            d->m_view->setPosition(value);
    }
}

void KMidPart::slotSeek(int value)
{
    seek(value);
}

void KMidPart::pause()
{
    if (d->m_midiobj != 0) {
        if (state() == Pause)
            d->m_midiobj->play();
        else
            d->m_midiobj->pause();
    }
}

void KMidPart::play()
{
    QMutexLocker locker(&d->m_connmutex);
    if (d->m_midiobj != NULL) {
        d->m_midiobj->play();
        d->m_playPending = false;
    }
}

void KMidPart::stop()
{
    if (d->m_midiobj != 0)
        d->m_midiobj->stop();
}

void KMidPart::slotSourceChanged(QString src)
{
    QMutexLocker locker(&d->m_connmutex);
    if (d->m_view != 0)
        d->m_view->setPosition(0);
    if (d->m_autoStart) {
        if (d->m_playerReady) {
            locker.unlock();
            play();
        } else
            d->m_playPending = true;
    }
    emit sourceChanged(src);
}

bool KMidPart::autoStart()
{
    return d->m_autoStart;
}

QString KMidPart::midiConnection()
{
    if (d->m_midiout != 0)
        return d->m_midiout->outputDeviceName();
    return QString();
}

void KMidPart::setAutoStart(bool start)
{
    d->m_autoStart = start;
}

void KMidPart::setMidiConnection(const QString conn)
{
    if (d->m_midiout != 0) {
        if (d->m_midiout->setOutputDeviceName(conn))
            d->m_settings->setOutput_connection(conn);
    }
}

void KMidPart::reload()
{
    if (d->m_midiobj != 0) {
        if (state() == Play)
            stop();
        qlonglong curpos = position();
        QString cursrc = d->m_midiobj->currentSource();
        if (!cursrc.isEmpty()) {
            d->m_midiobj->clear();
            d->m_midiobj->setCurrentSource(cursrc);
            seek(curpos);
        }
    }
}

QStringList KMidPart::metaData(const QString& key)
{
    if (d->m_midiobj != 0)
        return d->m_midiobj->metaData(key);
    return QStringList();
}

void KMidPart::setTempoFactor(double f)
{
    if (d->m_midiobj != 0)
        d->m_midiobj->setTimeSkew(f);
}

void KMidPart::setVolumeFactor(double f)
{
    d->m_volfactor = f;
    if (d->m_midiout != 0)
        d->m_midiout->setVolume(-1, f);
}

void KMidPart::setTranspose(int t)
{
    if (d->m_midiout != 0)
        d->m_midiout->setPitchShift(t);
}

double KMidPart::tempoFactor()
{
    if (d->m_midiobj != 0)
        return d->m_midiobj->timeSkew();
    return 1.0;
}

double KMidPart::volumeFactor()
{
    return d->m_volfactor;
}

int KMidPart::transpose()
{
    if (d->m_midiout != 0)
        return d->m_midiout->pitchShift();
    return 0;
}

void KMidPart::setMuted(int channel, bool muted)
{
    if (d->m_midiout != 0)
        d->m_midiout->setMuted(channel, muted);
}

bool KMidPart::isMuted(int channel)
{
    if (d->m_midiout != 0)
        return d->m_midiout->isMuted(channel);
    return false;
}

bool KMidPart::openUrl(const QUrl& url)
{
    return KParts::ReadOnlyPart::openUrl(url);
}

bool KMidPart::openUrl(const QString& str)
{
    QUrl url(str);
    return KParts::ReadOnlyPart::openUrl(url);
}

QString KMidPart::currentSource() const
{
    if (d->m_midiobj != 0)
        return d->m_midiobj->currentSource();
    return QString();
}

void KMidPart::slotTempoChanged(qreal tempo)
{
    emit tempoEvent(tempo);
}

QDBusVariant KMidPart::songProperty(const QString& key)
{
    if (d->m_midiobj != 0)
        return QDBusVariant(d->m_midiobj->songProperty(key));
    return QDBusVariant();
}

QDBusVariant KMidPart::channelProperty(int channel, const QString& key)
{
    if (d->m_midiobj != 0)
        return QDBusVariant(d->m_midiobj->channelProperty(channel, key));
    return QDBusVariant();
}
#include "kmid_part.moc"
