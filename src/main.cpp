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

#include <config.h>
#include "kmid2.h"
#include <kapplication.h>
#include "KF5/KDELibs4Support/k4aboutdata.h"
#include <kcmdlineargs.h>
#include <KDE/KLocale>
#include <QUrl>

static const char description[] =
    I18N_NOOP("A KDE 4 MIDI/Karaoke player");

static const char version[] = VERSION;

int main(int argc, char **argv)
{
    K4AboutData about( "kmid", 0, ki18n("KMid"), version, ki18n(description),
                      K4AboutData::License_GPL );
    about.addAuthor( ki18n("Pedro Lopez-Cabanillas"), ki18n("Maintainer"),
                     "plcl@users.sf.net" );
    about.addAuthor( ki18n("Antonio Larrosa Jimenez"), ki18n("Author"),
                     "larrosa@kde.org" );
    KCmdLineArgs::init(argc, argv, &about);
    KCmdLineOptions options;
    options.add("+[URL]", ki18n( "Song(s) to open" ));
    KCmdLineArgs::addCmdLineOptions(options);
    KApplication app;

    // see if we are starting with session management
    if (app.isSessionRestored()) {
        kRestoreMainWindows<KMid2>();
    } else {
        // no session.. just start up normally
        KMid2 *widget = new KMid2;
        KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
        if (args->count() > 0) {
            QList<QUrl> urls;
            for (int i = 0; i < args->count(); ++i)
                urls.append(args->url(i));
            args->clear();
            widget->setUrlsLater(urls);
        }
        widget->show();
    }
    return app.exec();
}
