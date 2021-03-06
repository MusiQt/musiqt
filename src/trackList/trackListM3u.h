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

#ifndef TRACKLISTM3U_H
#define TRACKLISTM3U_H

#include "trackListBackend.h"

#include <cctype>

class trackListM3u final : public trackListBackend
{
public:
    trackListM3u(const QString &path) : trackListBackend(path) {}

    /// Load playlist
    tracks_t load() override
    {
        constexpr int bufSize = 2048;
        char buffer[bufSize+1];
        buffer[bufSize]='\0';

        QFile file(m_path);
        if (!file.open(QIODevice::ReadOnly|QIODevice::Text))
            return QStringList();

        tracks_t tracks;
        while (const int n=file.read(buffer, bufSize))
        {
            if (buffer[0]!='#')
            {
                int i = 1;
                while (i<n && isprint(buffer[i])) i++;
                buffer[i] = '\0';
                tracks.append(QString(buffer));
            }
        }

        file.close();

        return tracks;
    }

    bool save(const tracks_t& tracks) override
    {
        QFile file(m_path);
        if (!file.open(QIODevice::WriteOnly|QIODevice::Text|QIODevice::Truncate))
            return false;

        QTextStream outputStream(&file);

        for (QString track : tracks)
        {
            writeLine(outputStream, track);
        }

        file.close();
        return true;
    }
};

#endif
