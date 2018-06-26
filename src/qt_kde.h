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

#ifndef QT_KDE_H
#define QT_KDE_H

#include <config.h>

// ALSA
#ifdef WITH_ALSA
#include <alsa/asoundlib.h>
#endif

// Standard C++ library
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <limits>

// Qt4 headers
#include <QtCore/QByteArray>
#include <QtCore/QDataStream>
#include <QtCore/QDebug>
#include <QtCore/QEvent>
#include <QtCore/QFile>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QMutex>
#include <QtCore/QObject>
#include <QtCore/QPointer>
#include <QtCore/QReadWriteLock>
#include <QtCore/QSignalMapper>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QtDebug>
#include <QtCore/QTextCodec>
#include <QtCore/QTextDecoder>
#include <QtCore/QTextEncoder>
#include <QtCore/QTextStream>
#include <QtCore/QThread>
#include <QtCore/QTime>
#include <QtCore/QTimerEvent>
#include <QtCore/QUrl>
#include <QtCore/QVariant>
#include <QtCore/QVariantList>
#include <QtCore/QVector>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDockWidget>
#include <QtGui/QDropEvent>
#include <QtGui/QFont>
#include <QtGui/QFrame>
#include <QtGui/QGraphicsRectItem>
#include <QtGui/QGraphicsScene>
#include <QtGui/QGraphicsSceneMouseEvent>
#include <QtGui/QGraphicsTextItem>
#include <QtGui/QGraphicsView>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QIcon>
#include <QtGui/QKeyEvent>
#include <QtGui/QLabel>
#include <QtGui/QListView>
#include <QtGui/QPainter>
#include <QtGui/QPalette>
#include <QtGui/QPen>
#include <QtGui/QPixmap>
#include <QtGui/QPrintDialog>
#include <QtGui/QPrinter>
#include <QtGui/QResizeEvent>
#include <QtGui/QScrollBar>
#include <QtGui/QSpacerItem>
#include <QtGui/QTextCursor>
#include <QtGui/QTextDocument>
#include <QtGui/QToolButton>
#include <QtGui/QToolTip>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>
#include <QtSvg/QSvgRenderer>

// KDE4 headers
#include "KF5/KDELibs4Support/k4aboutdata.h"
#include <kactioncollection.h>
#include <kaction.h>
#include <kapplication.h>
#include <kcharsets.h>
#include <kcmdlineargs.h>
#include <kcolorbutton.h>
#include <kcombobox.h>
#include <kcompletion.h>
#include <kcomponentdata.h>
#include <kcompositejob.h>
#include <kconfigdialog.h>
#include <kconfiggroup.h>
#include <kconfig.h>
#include <kconfigskeleton.h>
#include <kcoreconfigskeleton.h>
#include <kdebug.h>
#include <kdecore_export.h>
#include <kdemacros.h>
#include <kdeui_export.h>
#include <kdeversion.h>
#include <kdialog.h>
#include <keditlistbox.h>
#include <kexportplugin.h>
#include <kfiledialog.h>
#include <kfile.h>
#include <kfontcombobox.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <khbox.h>
#include <kiconloader.h>
#include <kinputdialog.h>
#include <kio/global.h>
#include <kio/jobclasses.h>
#include <kio/kio_export.h>
#include <kio/netaccess.h>
#include <kio/udsentry.h>
#include <kjob.h>
#include <kled.h>
#include <klibloader.h>
#include <klibrary.h>
#include <klineedit.h>
#include <klistwidget.h>
#include <klocale.h>
#include <kmainwindow.h>
#include <kmenubar.h>
#include <kmessagebox.h>
#include <knuminput.h>
#include <kpagedialog.h>
#include <kpagemodel.h>
#include <kpageview.h>
#include <kpagewidget.h>
#include <kpagewidgetmodel.h>
#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <kpushbutton.h>
#include <ksavefile.h>
#include <kserviceaction.h>
#include <kservice.h>
#include <kservicetypetrader.h>
#include <ksharedconfig.h>
#include <kstandardaction.h>
#include <kstandarddirs.h>
#include <kstandardguiitem.h>
#include <kstatusbar.h>
#include <ksycocaentry.h>
#include <ktextedit.h>
#include <ktoggleaction.h>
#include <kurl.h>
#include <kurlrequester.h>
#include <kxmlguiclient.h>
#include <kxmlguiwindow.h>

#endif /*QT_KDE_H*/
