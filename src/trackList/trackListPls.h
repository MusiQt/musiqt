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

#ifndef TRACKLISTPLS_H
#define TRACKLISTPLS_H

#include "trackListBackend.h"

#include <QSettings>

class trackListPls final : public trackListBackend
{
public:
    trackListPls(const QString &path) : trackListBackend(path) {}

    /// Load playlist
    tracks* load() override
    {
        QSettings pls(_path, QSettings::IniFormat);

        const int items = pls.value("playlist/NumberOfEntries").toInt();

        for (int i=1; i<=items; i++)
        {
            QString section = QString("playlist/File%1").arg(i);
            const QString file = pls.value(section).toString();
            _tracks->append(file);
        }

        return _tracks;
    }

    bool save(const tracks* tracks) override
    {
        QFile file(_path);
        if (!file.open(QIODevice::WriteOnly|QIODevice::Text|QIODevice::Truncate))
            return false;

        QTextStream outputStream(&file);

        outputStream << "[playlist]";

        QString tmp = QString("NumberOfEntries=%1").arg(tracks->size());
        outputStream << tmp;


        for (int i=0; i<tracks->size(); i++)
        {
            tmp = QString("File%1=%2").arg(i+1).arg(tracks->at(i).location());
            outputStream << tmp;
        }

        file.close();
        return true;
    }
};

#endif
