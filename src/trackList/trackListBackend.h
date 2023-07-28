/*
 *  Copyright (C) 2009-2023 Leandro Nini
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
#include <QFileInfo>
#include <QTextStream>

class trackListBackend : public trackList
{
protected:
    const QString m_path;

private:
    trackListBackend();
    trackListBackend(const trackListBackend&) = delete;
    trackListBackend& operator= (const trackListBackend&) = delete;

protected:
    trackListBackend(const QString &path) : m_path(path) {}

    static void writeLine(QTextStream& outputStream, QString line)
    {
        outputStream << line << '\n';
    }

    QString getAbsolutePath(const QString &file)
    {
        QFileInfo fileInfo(file);
        if (fileInfo.isAbsolute())
            return file;

        fileInfo.setFile(m_path);
        return fileInfo.absolutePath().append("/").append(file);
    }

public:
    virtual ~trackListBackend() {}

    /// Load playlist
    virtual tracks_t load() override { return tracks_t(); }

    /// Save playlist
    virtual bool save(const tracks_t&) override { return false; }
};

#endif
