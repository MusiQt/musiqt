/*
 *  Copyright (C) 2009-2021 Leandro Nini
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
    QString m_path;

private:
    trackListBackend();
    trackListBackend(const trackListBackend&);
    trackListBackend& operator= (const trackListBackend&);

protected:
    trackListBackend(const QString &path) : m_path(path) {}

    void writeLine(QTextStream& outputStream, QString line)
    {
        outputStream << line << '\n';
    }

public:
    virtual ~trackListBackend() {}

    /// Load playlist
    virtual tracks_t load() override { return tracks_t(); }

    /// Save playlist
    virtual bool save(const tracks_t&) override { return false; }
};

#endif
