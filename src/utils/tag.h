/*
 *  Copyright (C) 2007-2019 Leandro Nini
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

    QByteArray *_img;

private:
    ///
    QString getString(const char* ptr, const char max=30);

    /// Read ID3v1 tag if present
    int getID3v1(char* buf);

    ///
    QString getID3v2Text(const char* buf, int size);

    ///
    int getID3v2_2Frame(char* buf);

    ///
    int getID3v2Frame(char* buf, int ver);

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
    const QString& title() const { return _title; }

    /// Get author
    const QString& artist() const { return _artist; }

    /// Get publisher info
    const QString& publisher() const { return _publisher; }

    /// Get year info
    const QString& year() const { return _year; }

    /// Get album
    const QString& album() const { return _album; }

    /// Get genre
    const QString& genre() const { return _genre; }

    /// Get comment
    const QString& comment() const { return _comment; }

    /// Get track number
    const QString& track() const { return _track; }

    /// Get image
    QByteArray* image() const { return _img; }
};

#endif
