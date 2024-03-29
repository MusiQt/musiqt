/*
 *  Copyright (C) 2013-2021 Leandro Nini
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

#include <QListView>
#include <QMouseEvent>

class playlist final : public QListView
{
    Q_OBJECT

private:
    playlist() {}
    playlist(const playlist&) = delete;
    playlist& operator=(const playlist&) = delete;

public:
    playlist(QWidget * parent) :
        QListView(parent)
    {}

    void mousePressEvent(QMouseEvent *event) override
    {
        // Avoid selecting item on right-click
        if (event->button() == Qt::RightButton)
            return;
        QListView::mousePressEvent(event);
    }
};

#endif
