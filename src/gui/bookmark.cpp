/*
 *  Copyright (C) 2006-2021 Leandro Nini
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "bookmark.h"

#include "iconFactory.h"

#include <QDebug>
#include <QDropEvent>
#include <QLabel>
#include <QMenu>
#include <QMimeData>
#include <QMessageBox>
#include <QUrl>
#include <QWidgetAction>

#define SECTION QString("Bookmarks")
#define KEY(i)  QString("Bookmarks/bm%2").arg(i)

void bookmark::load()
{
    QString bm;
    int i = 0;
    while (!(bm=settings.value(KEY(i++)).toString()).isEmpty())
        addItem(bm);
}

void bookmark::save()
{
    settings.beginGroup(SECTION);
    settings.remove("");
    settings.endGroup();

    const int n = count();
    if (!n)
        return;

    for (int i=0; i<n; i++)
        settings.setValue(KEY(i), item(i)->text());

    clear();
}

void bookmark::add(const QString& dirName)
{
    // TODO check if valid?
    if (findItems(dirName, Qt::MatchFixedString|Qt::MatchCaseSensitive).empty())
        addItem(dirName);
}

void bookmark::dropEvent(QDropEvent *event)
{
    QList<QUrl> urlList = event->mimeData()->urls();
    for (int i=0; i<urlList.size(); ++i)
    {
        QString url = urlList.at(i).toLocalFile();
        add(url);
    }
}

void bookmark::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls())
    {
        event->acceptProposedAction();
    }
    else
    {
        event->ignore();
    }
}

void bookmark::dragMoveEvent(QDragMoveEvent *event)
{
    event->acceptProposedAction();
}

void bookmark::dragLeaveEvent(QDragLeaveEvent *event)
{
    event->accept();
}

void bookmark::contextMenuEvent(QContextMenuEvent * event)
{
    QListWidgetItem *item = itemAt(event->pos());

    QMenu pane(this);
    QString dirName = (item != nullptr) ? item->text() : tr("No item selected");
    if (dirName.count() > 20)
        dirName = dirName.left(10) + "..." + dirName.right(10);

    QWidgetAction *wa = new QWidgetAction(&pane);
    wa->setDefaultWidget(new QLabel(dirName));
    pane.addAction(wa);
    pane.addSeparator();

    QAction *delitem = pane.addAction(
        GET_ICON(icon_listremove), tr("Delete"), this,
        [this, item]()
        {
            const int idx = row(item);
            delete takeItem(idx);
        }
    );
    delitem->setStatusTip(tr("Delete selected bookmark"));
    if (item == nullptr)
        delitem->setEnabled(false);

    QAction *clearitem = pane.addAction(
        GET_ICON(icon_editdelete), tr("Clear"), this,
        [this]()
        {
            if (QMessageBox::question(
                this, tr("Clear bookmarks"),
                tr("Are you sure you want to delete all bookmarks?"),
                QMessageBox::Yes|QMessageBox::No) == QMessageBox::Yes)
            {
                clear();
            }
        }
    );
    clearitem->setStatusTip(tr("Clear all bookmarks"));
    if (count() == 0)
        clearitem->setEnabled(false);

    pane.exec(event->globalPos());
}
