/*
 *  Copyright (C) 2009-2017 Leandro Nini
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

#ifndef TRACKLIST_H
#define TRACKLIST_H

#include <QList>
#include <QString>
#include <QMetaType>

/*****************************************************************/

class track_t
{
private:
    QString _location;
    int _start;

public:
    track_t() : _location(QString::null), _start(0) {}
    track_t(const QString& location, const unsigned int start=0) : _location(location), _start(start) {}

    const QString& location() const { return _location; }

    unsigned int start() const { return _start; }
};

Q_DECLARE_METATYPE(track_t)

/*****************************************************************/

class tracks : public QList<track_t> // FIXME change to QAbstractListModel
{
public:
    tracks() : QList<track_t>() {}

    void append(const QString &text)
    {
        const track_t t(text);
        QList<track_t>::append(t);
    }
};

/*****************************************************************/

class trackList
{
public:
    virtual ~trackList() {}

    /// Load playlist
    virtual tracks* load() =0;

    /// Save playlist
    virtual bool save(const tracks*) =0;
};

#endif
