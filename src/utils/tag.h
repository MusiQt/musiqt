/*
 *  Copyright (C) 2007-2021 Leandro Nini
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
    QString m_title;
    QString m_artist;
    QString m_album;
    QString m_publisher;
    QString m_comment;
    QString m_genre;
    QString m_year;
    QString m_track;

    int m_offsBegin;
    int m_offsEnd;

    QByteArray *m_img;

private:
    /// Read ID3v1 tag if present
    int getID3v1(char* buf);

    ///
    int getID3v2_2Frame(char* buf);

    ///
    int getID3v2Frame(char* buf, int ver);

    ///
    int getExtHdrSize(const char* buf, int ver);

    ///
    bool checkID3v2(char* buf, bool header);

    ///
    bool parseID3v2header(char* buf, int& version, int& tagSize);

    ///
    int getLE32(const char* frame);

    ///
    bool getAPEItem(const char* orig, QString* dest, const char* tagName, int tagNameLen, int tagLen);

    ///
    int parseAPETag(const char* buf);

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
    int offsBegin() const { return m_offsBegin; }

    /// Get offset from end
    int offsEnd() const { return m_offsEnd; }

    /// Get title
    const QString& title() const { return m_title; }

    /// Get author
    const QString& artist() const { return m_artist; }

    /// Get publisher info
    const QString& publisher() const { return m_publisher; }

    /// Get year info
    const QString& year() const { return m_year; }

    /// Get album
    const QString& album() const { return m_album; }

    /// Get genre
    const QString& genre() const { return m_genre; }

    /// Get comment
    const QString& comment() const { return m_comment; }

    /// Get track number
    const QString& track() const { return m_track; }

    /// Get image
    QByteArray* image() const { return m_img; }
};

#endif
