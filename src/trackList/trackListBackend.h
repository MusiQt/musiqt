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

#ifndef TRACKLISTBACKEND_H
#define TRACKLISTBACKEND_H

#include "trackList.h"

#include <QFile>
#include <QTextStream>

class trackListBackend : public trackList
{
protected:
    tracks *_tracks;
    QString _path;

private:
    trackListBackend();
    trackListBackend(const trackListBackend&);
    trackListBackend& operator= (const trackListBackend&);

protected:
    trackListBackend(const QString &path) : _path(path)
    {
        _tracks = new tracks();
    }

    void writeLine(QFile* file, QString line)
    {
        QTextStream outputStream(file);
        //line.append(ENDLINE);
        outputStream << line;
    }

public:
    virtual ~trackListBackend() {}

    /// Load playlist
    virtual tracks* load() { return nullptr; }

    /// Save playlist
    virtual bool save(const tracks*) { return false; }
};

#endif
