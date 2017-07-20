/*
 *  Copyright (C) 2006-2017 Leandro Nini
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

#include "playlist.h"
#include "utils.h"
#include "iconFactory.h"
#include "trackListFactory.h"

#include <memory>

#include <QAction>
#include <QActionGroup>
#include <QDebug>
#include <QDropEvent>
#include <QFileInfo>
#include <QLabel>
#include <QMenu>
#include <QMimeData>
#include <QUrl>
#include <QWidgetAction>

bool playlist::add(const QString& item)
{
    if (findItems(item, Qt::MatchFixedString|Qt::MatchCaseSensitive).empty())
    {
        qDebug() << "Add " << item << " to playlist";
        QListWidgetItem *witem = new QListWidgetItem(item);
        addItem(witem);
        setCurrentItem(witem);
        return true;
    }
    qDebug() << "Duplicate item";
    return false;
}

void playlist::dropEvent(QDropEvent *event)
{
    QList<QUrl> urlList = event->mimeData()->urls();
    for (auto&& urlItem : urlList)
    {
        QString url(urlItem.path());
        if (QFileInfo(url).isFile())
        {
            add(url);
            //sortItems(order);
        }
    }

    event->acceptProposedAction();
}

void playlist::dragEnterEvent(QDragEnterEvent *event)
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

void playlist::dragMoveEvent(QDragMoveEvent *event)
{
    event->acceptProposedAction();
}

void playlist::dragLeaveEvent(QDragLeaveEvent *event)
{
    event->accept();
}

void playlist::contextMenuEvent(QContextMenuEvent * event)
{
    QListWidgetItem *item = itemAt(event->pos());
    setProperty("UserData", QVariant(row(item)));

    QMenu pane(this);
    if (item != nullptr)
    {
        QWidgetAction *wa = new QWidgetAction(&pane);
        QLabel *label = new QLabel(shrink(item->text()));
        label->setAlignment(Qt::AlignCenter);
        wa->setDefaultWidget(label);
        pane.addAction(wa);
        pane.addSeparator();
        QAction *delitem = pane.addAction(GET_ICON(icon_listremove), tr("Remove item"), this, SLOT(onCmdDel()));
        delitem->setStatusTip(tr("Remove selected item from playlist"));
        pane.addSeparator();
    }

    QAction* asc = new QAction(tr("Sort ascending"), &pane);
    asc->setCheckable(true);
    asc->setStatusTip(tr("Sort ascending"));
    if (ordered && order == Qt::AscendingOrder) asc->setChecked(true);
    connect(asc, SIGNAL(triggered()), this, SLOT(sortAsc()));
    QAction* desc = new QAction(tr("Sort descending"), &pane);
    desc->setCheckable(true);
    desc->setStatusTip(tr("Sort descending"));
    if (ordered && order == Qt::DescendingOrder) desc->setChecked(true);
    connect(desc, SIGNAL(triggered()), this, SLOT(sortDesc()));
    QAction* rnd = new QAction(tr("Shuffle"), &pane);
    rnd->setCheckable(true);
    rnd->setStatusTip(tr("Sort randomly"));
    if (!ordered) rnd->setChecked(true);
    connect(rnd, SIGNAL(triggered()), this, SLOT(shuffle()));

    QActionGroup *radioGroup = new QActionGroup(&pane);
    radioGroup->addAction(asc);
    radioGroup->addAction(desc);
    radioGroup->addAction(rnd);

    pane.addSeparator()->setText(tr("Sorting"));
    pane.addAction(asc);
    pane.addAction(desc);
    pane.addAction(rnd);

    pane.exec(event->globalPos());
}

void playlist::onCmdDel()
{
    int row = property("UserData").toInt();
    QListWidgetItem *item = takeItem(row);
    delete item;

    emit changed();
}

void playlist::sortAsc()
{
    order = Qt::AscendingOrder;
    ordered = true;
    sortItems(order);
}

void playlist::sortDesc()
{
    order = Qt::DescendingOrder;
    ordered = true;
    sortItems(order);
}

void playlist::shuffle()
{
    ordered = false;

    const int c = count();
    QList<QListWidgetItem*> items;
    items.reserve(c);
    for (int i=0; i<c; i++)
    {
        QListWidgetItem *it = takeItem(0);
        if (qrand()%2)
            items.append(it);
        else
            items.prepend(it);
    }

    for (auto item : items)
    {
        addItem(item);
    }

    emit changed();
}

int playlist::load(const QString& path)
{
    delPtr(_tracks);

    std::unique_ptr<trackList> tracklist(TFACTORY->get(path));

    if (tracklist.get() == nullptr)
        return 0;

    _tracks = tracklist->load();

    return _tracks->count();
}

bool playlist::save(const QString& file)
{
    std::unique_ptr<trackList> tracklist(TFACTORY->get(file));

    if (tracklist.get() == nullptr)
        return false;

    //FIXME
    if (_tracks == nullptr)
        _tracks = new tracks();
    else
        _tracks->clear();

    for (int i=0; i<count(); i++)
    {
        _tracks->append(item(i)->toolTip());
    }

    return tracklist->save(_tracks);
}

int playlist::filter(const QStringList& filter)
{
    if (_tracks == nullptr)
        return 0;

    QString filt(filter.join("|"));
    filt.prepend(".*\\.(").append(")");
    qDebug() << "filter: " << filt;

    clear();

    QRegExp regexp(filt);

    for (int i=0; i<_tracks->count(); i++)
    {
        QFileInfo location(_tracks->at(i).location());
        qDebug() << "File: " << location.absoluteFilePath();
        if (regexp.exactMatch(location.fileName())
            && location.exists())
        {
            QVariant qv;
            qv.setValue(_tracks->at(i));
            QListWidgetItem* item = new QListWidgetItem(location.completeBaseName());
            item->setData(Qt::UserRole, qv);
            item->setToolTip(location.absoluteFilePath());
            addItem(item);
        }
    }

    emit changed();

    return count();
}

void playlist::mousePressEvent(QMouseEvent *event)
{
    // Avoid selecting item on right-click
    if (event->button() == Qt::RightButton)
        return;
    QListWidget::mousePressEvent(event);
}
