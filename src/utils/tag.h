/*
 *  Copyright (C) 2007-2017 Leandro Nini
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

#ifndef TAG_H
#define TAG_H

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "imageData.h"

#include <QFile>

class tag
{
private:
    QString _title;
    QString _artist;
    QString _album;
    QString _publisher;
    QString _comment;
    QString _genre;
    QString _year;
    QString _track;

    int _offsBegin;
    int _offsEnd;

    imageData *_img;

private:
    ///
    QString getString(const char* ptr, const char max=30);

    /// Read ID3v1 tag if present
    int getID3v1(char* buf);

    ///
    QString getID3v2Text(const char* buf, int size);

    ///
    int getID3v2_2Frame(const char* buf);

    ///
    int getID3v2Frame(const char* buf, int ver);

    ///
    int getExtHdrSize(const char* buf, int ver);

    ///
    bool checkID3v2(char* buf, bool prepend);

    ///
    bool parseID3v2header(char* buf, int& version, int& tagSize);

    ///
    int getLE32(const char* frame);

    ///
    bool getComment(const char* orig, QString* dest, const char* type, int& len);

    ///
    int getAPEItem(const char* buf);

    ///
    bool checkAPE(char* buf, int& itemsSize, int& tagSize);

public:
    /**
     * Check if buffer contains a frame
     * \param buf       Pointer to the buffer
     * \param frame     Type of frame we want to check
     */
    static bool isFrame(const char* buf, const char* frame);

    /**
     *
     * \param frame
     * \param synchsafe
     * \return Size of frame
     */
    static int getFrameSize(const char* frame, bool synchsafe);

public:
    tag(QFile* file);
    ~tag() {}

    /// Get offset from beginning
    int offsBegin() const { return _offsBegin; }

    /// Get offset from end
    int offsEnd() const { return _offsEnd; }

    /// Get title
    const char* title() const { return _title.toStdString().c_str(); }

    /// Get author
    const char* artist() const { return _artist.toStdString().c_str(); }

    /// Get publisher info
    const char* publisher() const { return _publisher.toStdString().c_str(); }

    /// Get year info
    const char* year() const { return _year.toStdString().c_str(); }

    /// Get album
    const char* album() const { return _album.toStdString().c_str(); }

    /// Get genre
    const char* genre() const { return _genre.toStdString().c_str(); }

    /// Get comment
    const char* comment() const { return _comment.toStdString().c_str(); }

    /// Get track number
    const char* track() const { return _track.toStdString().c_str(); }

    /// Get image
    imageData* image() const { return _img; }
};

#endif
