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

#ifndef TRACKLISTCUE_H
#define TRACKLISTCUE_H

#include "trackListBackend.h"

extern "C" {
#  include <libcue/libcue.h>
}

#include <QDebug>

class trackListCue final : public trackListBackend
{
public:
    trackListCue(const QString &path) : trackListBackend(path) {}

    /// Load playlist
    tracks* load() override
    {
        FILE* file = fopen(_path.text(), "r");
        Cd* cue = cue_parse_file(file);  //Cd* cue_parse_string(const char*);
        fclose(file);

        const int tracks = cd_get_ntrack(cue);
        qDebug() << "tracks: " << tracks;

        for (int num_track=1; num_track<=tracks; num_track++)
        {
            Track* track = cd_get_track(cue, num_track);
        
            char* fileName = track_get_filename(track);
            //long start=track_get_start(track);
            //long length=track_get_length(track);
            qDebug() << "File: " << fileName;
            _tracks->append(FXPath::absolute(FXPath::directory(_path), fileName));
        }

        return _tracks;
    }
};

#endif
