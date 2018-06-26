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

#include "kmidtest.h"
#include "kmidtest.moc"

#include <KMediaPlayer/View>
#include <KPluginFactory>
#include <KPluginLoader>
#include <KFileDialog>
#include <KStandardAction>
#include <KActionCollection>
#include <KMessageBox>
#include <KApplication>
#include <KUrl>
#include <KAboutData>
#include <KCmdLineArgs>
#include <KCmdLineOptions>
#include <KLocale>

KMidTest::KMidTest()
    : KParts::MainWindow()
{
    KPluginFactory *factory = KPluginLoader("kmid_part").factory();
    if (factory) {
        m_part = factory->create<KMediaPlayer::Player>(this);
        if (m_part) {
            setCentralWidget(m_part->view());
            setupActions();
            setupGUI();
            createGUI(m_part);
            //m_part->view()->setButtons(KMediaPlayer::View::All);
        }
    } else {
        KMessageBox::error(this, i18n("Could not find KMidPart!"));
        kapp->quit();
        return;
    }
}

KMidTest::~KMidTest()
{ }

void KMidTest::setupActions()
{
    KStandardAction::open(this, SLOT(fileOpen()), actionCollection());
    KStandardAction::quit(kapp, SLOT(closeAllWindows()), actionCollection());
    createStandardStatusBarAction();
    setStandardToolBarMenuEnabled(true);
}

void KMidTest::fileOpen()
{
    KUrl url = KFileDialog::getOpenUrl(KUrl(),QLatin1String("audio/midi"),this);
    if (!url.isEmpty())
        m_part->openUrl(url);
}

void KMidTest::play(KUrl u)
{
    m_part->openUrl(u);
}

static const char description[] =
    I18N_NOOP("A KDE KPart Application");

static const char version[] = "0.1";

int main(int argc, char *argv[])
{
    KAboutData about("kmidtest", 0, ki18n("KMidTest"),
                     version, ki18n(description), KAboutData::License_GPL,
                     ki18n("(C) 2010 Pedro Lopez-Cabanillas"),
                     KLocalizedString(), 0, "plcl@users.sf.net" );
    about.addAuthor( ki18n("Pedro Lopez-Cabanillas"), KLocalizedString(),
                     "plcl@users.sf.net" );
    KCmdLineArgs::init(argc, argv, &about);
    KCmdLineOptions options;
    options.add("+[url]", ki18n( "Song to open" ));
    KCmdLineArgs::addCmdLineOptions(options);
    KApplication app;

    if (app.isSessionRestored())
        RESTORE(KMidTest)
    else {
        KMidTest *widget = new KMidTest;
        KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
        if (args != 0 && args->count() > 0)
            widget->play(args->url(0));
        widget->show();
    }
    return app.exec();
}
