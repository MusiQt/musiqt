/*
 *  Copyright (C) 2006-2019 Leandro Nini
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

#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <QListWidget>
#include <QTime>

#include "trackList.h"

class playlist : public QListWidget
{
    Q_OBJECT

private:
    static unsigned int _seed;

    tracks_t tracks;

    Qt::SortOrder order;
    bool ordered;

private:
    playlist() {}
    playlist(const playlist&);
    playlist& operator=(const playlist&);

public:
    playlist(QWidget * parent) :
        QListWidget(parent),
        tracks(nullptr),
        order(Qt::AscendingOrder),
        ordered(true)
    {
        setAcceptDrops(true);
        setDropIndicatorShown(true);
        setContextMenuPolicy(Qt::DefaultContextMenu);

        QTime now = QTime::currentTime();
        qsrand(now.msec());
    }

    /// Add item to playlist
    bool add(const QString& item);

    /// Load playlist
    int load(const QString& path);

    /// Save playlist
    bool save(const QString& file);

    /// Filter playlist
    int filter(const QStringList& filter);

    /// Get track location
    const QString getLocation(int index) const { return (item(index)->data(Qt::UserRole).value<track_t>()); }

protected:
    void dropEvent(QDropEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;

    void contextMenuEvent(QContextMenuEvent * event) override;

    void mousePressEvent(QMouseEvent *event) override;

signals:
    void changed();

protected slots:
    void onCmdDel();
    void sortAsc();
    void sortDesc();
    void shuffle();
};

#endif
