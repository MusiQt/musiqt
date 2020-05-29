/*
 *  Copyright (C) 2019-2020 Leandro Nini
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

#ifndef PLAYLISTMODEL_H
#define PLAYLISTMODEL_H

#include "trackListFactory.h"

#include <QAbstractListModel>
#include <QFileInfo>
#include <QStringList>
#include <QMimeData>
#include <QUrl>
#include <QDebug>

#include <memory>

#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
#  include <QRandomGenerator>

    int random() { return QRandomGenerator::global()->generate(); }
#else
    int random() { return qrand(); }
#endif

class playlistModel : public QAbstractListModel
{
    Q_OBJECT

private:
    QStringList locations;

private:
    void setStringList(const QStringList &strings)
    {
        beginResetModel();
        locations = strings;
        endResetModel();
    }

public:
    explicit playlistModel(QObject *parent=0) : QAbstractListModel(parent) {}

    int rowCount(const QModelIndex &parent = QModelIndex()) const override
    {
        return parent.isValid() ? 0 : locations.size();
    }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override
    {
        if (!index.isValid())
            return QVariant();

        const QFileInfo location(locations.value(index.row()));

        if (role == Qt::DisplayRole)
            return location.completeBaseName();

        if (role == Qt::ToolTipRole)
            return location.absoluteFilePath();

        if (role == Qt::UserRole)
            return locations.value(index.row());

        if (role == Qt::UserRole+1)
            return location.fileName();

        return QVariant();
    }

    Qt::ItemFlags flags(const QModelIndex &index) const override
    {
        if (!index.isValid())
            return Qt::ItemIsDropEnabled;

        return Qt::ItemIsDropEnabled|Qt::ItemIsEnabled|Qt::ItemIsSelectable
#if QT_VERSION >= 0x050000
            |Qt::ItemNeverHasChildren
#endif
            ;
    }

    void clear()
    {
        beginResetModel();
        locations.clear();
        endResetModel();
    }

    void append(QString data)
    {
        beginInsertRows(QModelIndex(), locations.size(), locations.size()+1);
        locations.append(data);
        endInsertRows();
    }

    void load(const QString& path)
    {
        std::unique_ptr<trackList> tracklist(TFACTORY->get(path));

        if (tracklist.get() == nullptr)
            clear();
        else
            setStringList(tracklist->load());
    }

    void shuffle()
    {
        QStringList items;
        items.reserve(locations.size());
        for (auto i : locations)
        {
            if (random()%2)
                items.append(i);
            else
                items.prepend(i);
        }

        setStringList(items);
    }

#if QT_VERSION >= 0x050000
    bool canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const override
    {
        return data->hasUrls();
    }
#endif

    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override
    {
        if (action == Qt::IgnoreAction)
            return true;

#if QT_VERSION < 0x050000
        if (!data->hasUrls())
            return false;
#endif

        int beginRow;
        
        if (row != -1)
            beginRow = row;
        else if (parent.isValid())
            beginRow = parent.row();
        else
            beginRow = rowCount(QModelIndex());

        QList<QUrl> urlList = data->urls();
        int rows = urlList.size();

        insertRows(beginRow, rows, QModelIndex());

        for (auto&& urlItem : urlList)
        {
            QString url(urlItem.toLocalFile());
            if (QFileInfo(url).isFile())
            {
                qDebug() << "adding url " << url;
                QModelIndex idx = index(beginRow, 0, QModelIndex());
                setData(idx, url);
                beginRow++;
            }
        }

        return true;
    }

    bool insertRows(int row, int count, const QModelIndex & parent = QModelIndex()) override
    {
        beginInsertRows(QModelIndex(), row, row+count-1);
        locations.append("---");
        endInsertRows();
        return true;
    }

    bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole) override
    {
        locations.replace(index.row(), value.toString());
        emit dataChanged(index, index);
        return true;
    }

    QStringList mimeTypes() const override
    {
        QStringList types;
        types << QLatin1String("text/uri-list");
        return types;
    }
};

#endif
