#!/usr/bin/env python
# -*- coding: utf-8 -*-

#
#   KMid2 MIDI/Karaoke Player
#   Copyright (C) 2009-2010 Pedro Lopez-Cabanillas <plcl@users.sf.net>
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License along
#   with this program; if not, write to the Free Software Foundation, Inc.,
#   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
#

from PyKDE4.kdecore import *
from PyKDE4.kdeui import *
from PyKDE4.kio import *
from PyKDE4.kparts import *
from PyQt4.QtCore import *
import sys


class MainWindow (KParts.MainWindow):
    def __init__ (self, app):
        KParts.MainWindow.__init__(self)
        self.app = app
        self.resize(330, 130)
        factory = KPluginLoader("kmid_part").factory()
        if (factory):
            part = factory.create(self, "KMidPart")
            if (part):
                self.part = part
                self.setCentralWidget(part.widget())
                self.act('file_open', KStandardAction.Open, self.openDocument)
                self.act('file_quit', KStandardAction.Quit, self.app.quit)
                self.setStandardToolBarMenuEnabled(True)
                self.setupGUI()
                self.createGUI(part)
        
    def openDocument(self):
        url = KFileDialog.getOpenUrl(KUrl(), "audio/midi", self, "Open File")
        if not url.isEmpty():
            self.part.openUrl(url)
            
    def act(self, name, texttype, func):
        a = self.actionCollection().addAction(texttype, name)
        QObject.connect(a, SIGNAL("triggered()"), func)
        return a

#--------------- main ------------------
if __name__ == '__main__':
    appName     = "kmidtest"
    catalog     = ""
    programName = ki18n("KMid Part Example")
    version     = "0.1"
    description = ki18n("Example loading KMidPart")
    license     = KAboutData.License_GPL
    copyright   = ki18n("(c) 2010 Pedro Lopez-Cabanillas")
    text        = ki18n("KDE4 MIDI/Karaoke player")
    homePage    = "http://kmid2.sourceforge.net"
    bugEmail    = "plcl@users.sf.net"
    
    aboutData   = KAboutData(appName, catalog, programName, version, description,
                                license, copyright, text, homePage, bugEmail)
           
    KCmdLineArgs.init(sys.argv, aboutData)
        
    app = KApplication()
    mainWindow = MainWindow(app)
    mainWindow.show()
    app.exec_()
