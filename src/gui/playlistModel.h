/*
 *  Copyright (C) 2019-2021 Leandro Nini
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

#include <QStringListModel>
#include <QFileInfo>
#include <QStringList>
#include <QMimeData>
#include <QUrl>
#include <QDebug>

class playlistModel : public QStringListModel
{
public:
    explicit playlistModel(QObject *parent=0) : QStringListModel(parent) {}

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override
    {
        if (!index.isValid())
            return QVariant();

        const QFileInfo location(stringList().value(index.row()));

        if (role == Qt::DisplayRole)
            return location.completeBaseName();

        if (role == Qt::ToolTipRole)
            return location.absoluteFilePath();

        if (role == Qt::UserRole)
            return stringList().value(index.row());

        if (role == Qt::UserRole+1)
            return location.fileName();

        return QVariant();
    }

    Qt::ItemFlags flags(const QModelIndex &index) const override
    {
        if (!index.isValid())
            return QAbstractListModel::flags(index) | Qt::ItemIsDropEnabled;

        return QAbstractListModel::flags(index) | Qt::ItemIsDropEnabled | Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemNeverHasChildren;
    }

    void clear()
    {
        setStringList(QStringList());
    }

    void append(QString data)
    {
        const int rows = rowCount();
        insertRow(rows);
        setData(index(rows, 0), data);
    }

    bool canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const override
    {
        Q_UNUSED(action)
        Q_UNUSED(row)
        Q_UNUSED(column)
        Q_UNUSED(parent)

        return data->hasUrls();
    }

    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, [[maybe_unused]] int column, const QModelIndex &parent) override
    {
        if (action == Qt::IgnoreAction)
            return true;

        int beginRow;

        if (row != -1)
            beginRow = row;
        else if (parent.isValid())
            beginRow = parent.row();
        else
            beginRow = rowCount();

        QList<QUrl> urlList = data->urls();
        int rows = urlList.size();

        insertRows(beginRow, rows);

        for (auto&& urlItem : urlList)
        {
            QString url(urlItem.toLocalFile());
            if (QFileInfo(url).isFile())
            {
                qDebug() << "adding url" << url;
                setData(index(beginRow, 0), url);
                beginRow++;
            }
        }

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
