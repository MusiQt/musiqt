/*
 *  Copyright (C) 2010-2021 Leandro Nini
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

#ifndef METADATA_H
#define METADATA_H

#include <QByteArray>
#include <QString>

class metaData
{
public:
    // https://www.freedesktop.org/wiki/Specifications/mpris-spec/metadata/
    enum mpris_t
    {
        // MPRIS-specific
        TRACK_ID,         // DBus path
        LENGTH,           // 64-bit integer
        ART_URL,          // URI
        // Common Xesam properties
        ALBUM,            // String
        ALBUM_ARTIST,     // List of Strings
        ARTIST,           // List of Strings
        AS_TEXT,          // String
        AUDIO_BPM,        // Integer
        AUTO_RATING,      // Float
        COMMENT,          // List of Strings
        COMPOSER,         // List of Strings
        CONTENT_CREATED,  // Date/Time
        DISC_NUMBER,      // Integer
        FIRST_USED,       // Date/Time
        GENRE,            // List of Strings
        LAST_USED,        // Date/Time
        LYRICIST,         // List of Strings
        TITLE,            // String
        TRACK_NUMBER,     // Integer
        URL,              // URI
        USER_COUNT,       // Integer
        USER_RATING,      // Float
        LAST_ID
    };

public:
    virtual int moreInfo(int i) const =0;
    virtual QString getKey(unsigned int num) const =0;
    virtual QString getKey(const mpris_t info) const =0;
    virtual QString getInfo(unsigned int num) const =0;
    virtual QString getInfo(const mpris_t info) const =0;
    virtual QString getInfo(const char* info) const =0;
    virtual QByteArray* getImage() const =0;

protected:
    ~metaData() {}
};

#endif
