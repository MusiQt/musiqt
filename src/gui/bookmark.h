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

#ifndef BOOKMARK_H
#define BOOKMARK_H

#include <QListWidget>
#include <QSettings>

class bookmark : public QListWidget
{
private:
    QSettings m_settings;

private:
    bookmark() {}
    bookmark(const bookmark&);
    bookmark& operator=(const bookmark&);

public:
    bookmark(QWidget * parent) :
        QListWidget(parent)
    {
        setAcceptDrops(true);
        setDropIndicatorShown(true);
        setContextMenuPolicy(Qt::DefaultContextMenu);
        //setSortingEnabled(true);
    }
    virtual ~bookmark() {}

    /// Add bookmark
    void add(const QString& dirName);

    /// Load bookmarks
    void load();

    /// Save and clear bookmarks
    void save();

public:
    void dropEvent(QDropEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;

    void contextMenuEvent(QContextMenuEvent * event) override;
};

#endif
