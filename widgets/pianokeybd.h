/*
    Virtual Piano Widget for Qt4 
    Copyright (C) 2008-2010 Pedro Lopez-Cabanillas <plcl@users.sf.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along 
    with this program; If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef PIANOKEYBD_H
#define PIANOKEYBD_H

#include "pianoscene.h"
#include <QGraphicsView>

#if defined(VPIANO_PLUGIN)
#include <QtDesigner/QDesignerExportWidget>
#else
#define QDESIGNER_WIDGET_EXPORT
#endif

class QDESIGNER_WIDGET_EXPORT PianoKeybd : public QGraphicsView
{
    Q_OBJECT
    Q_PROPERTY( int baseOctave READ baseOctave WRITE setBaseOctave )
    Q_PROPERTY( int numOctaves READ numOctaves WRITE setNumOctaves )
    Q_PROPERTY( int rotation READ getRotation WRITE setRotation )
    Q_PROPERTY( QColor keyPressedColor READ getKeyPressedColor WRITE setKeyPressedColor )
    Q_PROPERTY( bool showLabels READ showLabels WRITE setShowLabels )
    Q_PROPERTY( bool useFlats READ useFlats WRITE setUseFlats )
    Q_PROPERTY( int transpose READ getTranspose WRITE setTranspose )
#if defined(VPIANO_PLUGIN)
    Q_CLASSINFO("Author", "Pedro Lopez-Cabanillas <plcl@users.sf.net>")
    Q_CLASSINFO("URL", "http://sourceforge.net/projects/vmpk")
    Q_CLASSINFO("Version", "0.8")
#endif
public:
    PianoKeybd(QWidget *parent = 0);
    PianoKeybd(const int baseOctave, const int numOctaves, QWidget *parent = 0);
    virtual ~PianoKeybd();
    void setKeyboardMap(KeyboardMap* m) { m_scene->setKeyboardMap(m); }
    void resetKeyboardMap() { m_scene->setKeyboardMap(&m_defaultMap); }
    KeyboardMap* getKeyboardMap() { return m_scene->getKeyboardMap(); }

    int baseOctave() const { return m_scene->baseOctave(); }
    int numOctaves() const { return m_scene->numOctaves(); }
    void setBaseOctave(const int baseOctave) { m_scene->setBaseOctave(baseOctave); }
    void setNumOctaves(const int numOctaves);
    QSize sizeHint() const;
    QSize minimumSizeHint() const;
    int getRotation() const { return m_rotation; }
    void setRotation(int r);
    QColor getKeyPressedColor() const { return m_scene->getKeyPressedColor(); }
    void setKeyPressedColor(const QColor& c) { m_scene->setKeyPressedColor(c); }
    void allKeysOff() { m_scene->allKeysOff(); }
    void setPianoHandler(PianoHandler* handler) { m_scene->setPianoHandler(handler); }
    int minNote() const { return m_scene->getMinNote(); }
    void setMinNote(int n) { m_scene->setMinNote(n); }
    int maxNote() const { return m_scene->getMaxNote(); }
    void setMaxNote(int n) { m_scene->setMaxNote(n); }
    int getTranspose() const { return m_scene->getTranspose(); }
    void setTranspose(int t) { m_scene->setTranspose(t); }
    bool showLabels() const { return m_scene->showLabels(); }
    void setShowLabels(bool show) { m_scene->setShowLabels(show); }
    bool useFlats() const { return m_scene->useFlats(); }
    void setUseFlats(bool use) { m_scene->setUseFlats(use); }

public slots:
    void showNoteOn( int midiNote );
    void showNoteOff( int midiNote );

signals:
    void noteOn( int midiNote );
    void noteOff( int midiNote );

protected:
    void initialize();
    void initDefaultMap();
    void initScene(int base, int num, const QColor& c = QColor());
    void resizeEvent(QResizeEvent *event);

private:
    int m_rotation;
    PianoScene *m_scene;
    KeyboardMap m_defaultMap;
};

#endif // PIANOKEYBD_H
