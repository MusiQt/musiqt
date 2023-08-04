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

#ifndef TRACKLISTCUE_H
#define TRACKLISTCUE_H

#include "trackListBackend.h"

extern "C" {
#  include <libcue.h>
}

#include <QDebug>

class trackListCue final : public trackListBackend
{
public:
    trackListCue(const QString &path) : trackListBackend(path) {}

    /// Load playlist
    tracks_t* load() override
    {
        QFile file(m_path);
        if (!file.open(QIODevice::ReadOnly|QIODevice::Text))
            return QStringList();

        QByteArray line = file.readAll();
        file.close();

        Cd* cue = cue_parse_string(line.constData());
        if (!cue)
            return QStringList();

        //const char *val;
        //Cdtext *cdtext = cd_get_cdtext(cue);
        //val = cdtext_get(PTI_TITLE, cdtext);
        //val = cdtext_get(PTI_PERFORMER, cdtext);

        const int track_num = cd_get_ntrack(cue);
        qDebug() << "tracks: " << track_num;

        tracks_t tracks;
        for (int num_track=1; num_track<=track_num; num_track++)
        {
            Track* track = cd_get_track(cue, num_track);

            char* fileName = track_get_filename(track);
            //cdtext = track_get_cdtext(track);
            //val = cdtext_get(PTI_TITLE, cdtext);
            //long start=track_get_start(track);
            //long length=track_get_length(track);
            qDebug() << "File:" << fileName;
            tracks->append(FXPath::absolute(FXPath::directory(m_path), fileName));
        }

        return tracks;
    }
};

#endif
