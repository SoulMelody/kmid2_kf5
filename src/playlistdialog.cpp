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

#include "playlistdialog.h"
#include <QListView>
#include <KLocale>
#include <KUrlRequester>
#include <KEditListBox>
#include <KApplication>

PlayListDialog::PlayListDialog(QWidget *parent) : KDialog(parent)
{
    setCaption( i18nc("@title:window","Manage Playlist") );
    setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Reset | KDialog::Help );
    setHelp("playlists", KApplication::applicationName());
    //m_requester = new KUrlRequester(this); m_requester->customEditor(), 
    m_listBox = new KEditListBox(QString(), this);
    setMainWidget(m_listBox);
    connect(this, SIGNAL(resetClicked()), SLOT(reset()));
}

PlayListDialog::~PlayListDialog()
{ }

void PlayListDialog::reset()
{
    m_listBox->clear();
}

void PlayListDialog::setItems(const QStringList& items)
{
    m_listBox->setItems(items);
}

QStringList PlayListDialog::items() const
{
    return m_listBox->items();
}

void PlayListDialog::setCurrentItem(const QString& item)
{
    int row = m_listBox->items().indexOf(item);
    if (row > -1) {
        QAbstractItemModel *model = m_listBox->listView()->model();
        QModelIndex idx = model->index(row, 0, QModelIndex());
        m_listBox->listView()->setCurrentIndex(idx);
    }
}

QString PlayListDialog::currentItem() const
{
    return m_listBox->currentText();
}

int PlayListDialog::itemCount() const
{
    return m_listBox->count();
}
