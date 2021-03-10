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

#include "tag.h"

#include "genres.h"
#include "utils.h"

#include <QDebug>

#define ID3V1_TAG_SIZE 128

#define ID3V2_HEADER_SIZE 10
#define ID3V2_EXT_HEADER_SIZE 6

#define APE_HEADER_SIZE 32

tag::tag(QFile* file) :
    m_offsBegin(0),
    m_offsEnd(0),
    m_img(nullptr)
{
    char buf[ID3V1_TAG_SIZE];
    int id3v1Size = 0;

    // Check for an APE tag at the end of file
    // it may be confused as an ID3v1 tag
    file->seek(file->size()-APE_HEADER_SIZE);
    file->read(buf, 8);
    if (!isFrame(buf, "APETAGEX"))
    {
        // Check for ID3v1 tag
        file->seek(file->size()-ID3V1_TAG_SIZE);
        file->read(buf, ID3V1_TAG_SIZE);
        id3v1Size = getID3v1(buf);
    }

    // Check for ID3v2 tag
    file->seek(0);
    file->read(buf, ID3V2_HEADER_SIZE);

    int version = 0;
    int tagSize = 0;
    int extHdrSize = 0;
    if (checkID3v2(buf, true))
    {
        if (parseID3v2header(buf, version, tagSize))
        {
            qDebug() << "Extended header found";
            file->read(buf, ID3V2_EXT_HEADER_SIZE);
            extHdrSize = getExtHdrSize(buf, version);
            if (extHdrSize)
                file->seek(file->pos() + extHdrSize);
        }
    }
    else
    {
        file->seek(file->size()-(ID3V2_HEADER_SIZE+id3v1Size));
        file->read(buf, ID3V2_HEADER_SIZE);
        if (checkID3v2(buf, false))
        {
            if (parseID3v2header(buf, version, tagSize))
            {
                qDebug() << "Extended header found";
                file->read(buf, ID3V2_EXT_HEADER_SIZE);
                extHdrSize = getExtHdrSize(buf, version);
            }
            file->seek(file->pos() - (tagSize+ID3V2_EXT_HEADER_SIZE));
        }
    }

    int id3v2Size = 0;
    if (tagSize)
    {
        id3v2Size = ID3V2_HEADER_SIZE+tagSize;
        if (extHdrSize)
            id3v2Size += ID3V2_EXT_HEADER_SIZE+extHdrSize;

        char *dataBuf = new char[tagSize];
        file->read(dataBuf, tagSize);

        int i = 0;
        while ((i < tagSize) && dataBuf[i])
            i += (version == 2) ? getID3v2_2Frame(dataBuf+i)+6 : getID3v2Frame(dataBuf+i, version)+10;

        delete[] dataBuf;
    }

    if (id3v2Size)
        return;

    // Check for APE tag
    file->seek(0);
    file->read(buf, APE_HEADER_SIZE);

    int itemsSize = 0;
    if (!checkAPE(buf, itemsSize, tagSize))
    {
        file->seek(file->size()-(APE_HEADER_SIZE+id3v1Size));
        file->read(buf, APE_HEADER_SIZE);
        if (!checkAPE(buf, itemsSize, tagSize))
            return;
        file->seek(file->pos()-(APE_HEADER_SIZE+itemsSize));
    }

    char* dataBuf = new char[itemsSize];
    file->read(dataBuf, itemsSize);

    int i = 0;
    while ((i < itemsSize) && dataBuf[i])
        i += parseAPETag(dataBuf+i);

    delete[] dataBuf;
}

/************************/

bool tag::isFrame(const char* buf, const char* frame)
{
    for (unsigned int i=0; i<strlen(frame); i++)
    {
        if (buf[i] != frame[i])
            return false;
    }
    return true;
}

int tag::getFrameSize(const char* frame, bool synchsafe)
{
    return synchsafe
        ? ((unsigned int)((unsigned char)(frame[0]&0x7f))<<21) | ((unsigned int)((unsigned char)(frame[1]&0x7f))<<14)
            | ((unsigned int)((unsigned char)(frame[2]&0x7f))<<7) | ((unsigned int)((unsigned char)(frame[3]&0x7f)))
        : ((unsigned int)((unsigned char)frame[0])<<24) | ((unsigned int)((unsigned char)frame[1])<<16)
            | ((unsigned int)((unsigned char)frame[2])<<8) | ((unsigned int)((unsigned char)frame[3]));
}

/******** ID3v1 ********/

QString tag::getString(const char* ptr, const char max)
 {
    unsigned char len = 0;
    while (ptr[len] && (len < max))
        len++;
    return QString::fromLatin1(ptr, len).simplified();
}

int tag::getID3v1(char* buf)
{
    if (!isFrame(buf, "TAG"))
        return 0;

    qDebug() << "ID3v1 tag found.";

    if (m_title.isEmpty())
        m_title = getString(buf+3);

    if (m_artist.isEmpty())
        m_artist = getString(buf+33);

    if (m_album.isEmpty())
        m_album = getString(buf+63);

    if (m_year.isEmpty())
    {
        m_year = getString(buf+93, 4);
        if (m_year.length() < 4)
            m_year.resize(0);
    }

    if (m_comment.isEmpty())
        m_comment = getString(buf+97);

    unsigned char tmp = (unsigned char)buf[126];
    if (m_track.isNull() && !buf[125] && tmp)
        m_track = QString::number(tmp);

    tmp = (unsigned char)buf[127];
    if (m_genre.isEmpty() && tmp<GENRES)
        m_genre = QString(::genre[tmp]);

    return ID3V1_TAG_SIZE;
}

/******** ID3v2 ********/

QString tag::getID3v2Text(const char* buf, int size)
{
    // text encoding
    // 00 - ISO-8859-1
    // 01 - UTF-16 with BOM
    // 02 - UTF-16BE
    // 03 - UTF-8

    const char encoding = *buf++;
    qDebug() << "ID3v2 Frame encoding: " << encoding;

    QString tmp;
    switch (encoding)
    {
    case 0:
        tmp = QString::fromLatin1(buf, size);
        break;
    case 1:
        tmp = QString::fromUtf16((const ushort *)buf, size);
        break;
    case 2:
        {
            // Add BOM (0xfe 0xff)
            ushort* tempBuffer = new ushort[size*2 + 2];
            tempBuffer[0] = 0xfe;
            tempBuffer[1] = 0xff;
            memcpy(tempBuffer+2, buf, size*2);
            tmp = QString::fromUtf16(tempBuffer, size);
            delete [] tempBuffer;
        }
        break;
    case 3:
        tmp = QString::fromUtf8(buf, size);
        break;
    default:
        return QString();
    }

    tmp.replace('\0', ',');

    // Trim null char at the end of string
    return tmp.trimmed();
}

int tag::getID3v2_2Frame(char* buf)
{
    const int size = ((unsigned int)((unsigned char)buf[3])<<16) | ((unsigned int)((unsigned char)buf[4])<<8)
        | ((unsigned int)((unsigned char)buf[5]));
    qDebug() << "ID3v2 Frame: " << QString(buf).left(3) << " size: " << size;

    if (isFrame(buf, "TT2"))
    {
        m_title = getID3v2Text(buf+6, size-1);
        qDebug() << "ID3v2 title: " << m_title;
    }
    else if (isFrame(buf, "TP1"))
    {
        m_artist = getID3v2Text(buf+6, size-1);
        qDebug() << "ID3v2 artist: " << m_artist;
    }
    else if (isFrame(buf, "TAL"))
    {
        m_album = getID3v2Text(buf+6, size-1);
        qDebug() << "ID3v2 album: " << m_album;
    }
    else if (isFrame(buf, "TRK"))
    {
        QString t = getID3v2Text(buf+6, size-1);
        m_track = t.left(t.indexOf('/'));
        qDebug() << "ID3v2 track: " << m_track;
    }
    else if (isFrame(buf, "TYE"))
    {
        m_year = getID3v2Text(buf+6, size-1);
        qDebug() << "ID3v2 year: " << m_year;
    }
    else if (isFrame(buf, "TPB"))
    {
        m_publisher.append(getID3v2Text(buf+6, size-1));
        qDebug() << "ID3v2 publisher: " << m_publisher;
    }
    else if (isFrame(buf, "TCO"))
    {
        QString g = getID3v2Text(buf+6, size-1);
        QString n = g.mid(g.indexOf('('), g.indexOf(')'));
        m_genre=(g.indexOf('(')>=0) ? QString(::genre[n.toInt()]) : g;
        qDebug() << "ID3v2 genre: " << m_genre;
    }
    else if (isFrame(buf, "PIC"))
    {
        // Attached picture   "PIC"
        // Frame size         $xx xx xx
        // Text encoding      $xx
        // Image format       $xx xx xx
        // Picture type       $xx
        // Description        <textstring> $00 (00)
        // Picture data       <binary data>
        QString mime = QString(buf+11);
        qDebug() << "ID3v2 pic mime: " << mime;
        int i = mime.length();
        char type = buf[12+i];
        qDebug() << "Pic type: " << type;
        const QString description=QString(buf+13+i);
        qDebug() << "Pic description: " << description;
        const int j = description.length();
        const int imgOffset = 14+i+j;
        qDebug() << "imgOffset: " << imgOffset;
        m_img = new QByteArray(buf+imgOffset, size-imgOffset);
    }
    else
    /*if (isFrame(buf, "COM"))
    {
        int i = 10;
        while (*(buf+i))
            i++;
        m_comment = getID3v2Text(buf+i+1, size-i+9, buf[6]); // FIXME
        qDebug() << "ID3v2 comment: %s\n", m_comment.text()));
    }
    else*/
        qDebug() << "ID3v2 unhandled frame " << QString(buf).left(3);

    return size;
}

int tag::getID3v2Frame(char* buf, int ver)
{
    const int size = getFrameSize(buf+4, (ver == 4));
    qDebug() << "ID3v2 Frame: " << QString(buf).left(4) << " size: " << size;

    // Flags %0h00kmnp
    // h - Grouping identity
    // k - Compression
    // m - Encryption
    // n - Unsynchronisation
    // p - Data length indicator
    const char flags = buf[9];

    if (flags & 0x0E)
        return size;

    // Grouping identity
    if (flags & 0x40)
        buf++;

    // Data length indicator
    if (flags & 0x01)
        buf += 4;

    if (isFrame(buf, "TIT2"))
    {
        m_title = getID3v2Text(buf+10, size-1);
        qDebug() << "ID3v2 title: " << m_title;
    }
    else if (isFrame(buf, "TPE1"))
    {
        m_artist = getID3v2Text(buf+10, size-1);
        qDebug() << "ID3v2 artist: " << m_artist;
    }
    else if (isFrame(buf, "TALB"))
    {
        m_album = getID3v2Text(buf+10, size-1);
        qDebug() << "ID3v2 album: " << m_album;
    }
    else if (isFrame(buf, "TRCK"))
    {
        m_track = (getID3v2Text(buf+10, size-1));
        qDebug() << "ID3v2 track: " << m_track;
    }
    else if (isFrame(buf, "TYER"))
    {
        m_year = getID3v2Text(buf+10, size-1);
        qDebug() << "ID3v2 year: " << m_year;
    }
    else if (isFrame(buf, "TPUB"))
    {
        m_publisher = getID3v2Text(buf+10, size-1);
        qDebug() << "ID3v2 publisher: " << m_publisher;
    }
    else if (isFrame(buf, "TCON"))
    {
        QString g = getID3v2Text(buf+10, size-1);
        QString n = g.mid(g.indexOf('('), g.indexOf(')'));
        m_genre = (g.indexOf('(') >= 0) ? QString(::genre[n.toInt()]) : g;
        qDebug() << "ID3v2 genre: " << m_genre;
    }
    else
    /*if (isFrame(buf, "COMM"))
    {
        int i = 14;
        while (*(buf+i))
            i++;
        m_comment = getID3v2Text(buf+i+1, size-i+9, buf[10]); // FIXME
        qDebug() << "ID3v2 comment: %s\n", m_comment.text()));
    }
    else*/
    if (isFrame(buf, "APIC"))
    {
        QString mime = QString(buf+11);
        qDebug() << "ID3v2 pic mime: " << mime;
        const int i = mime.length();
        const char type = buf[12+i];
        qDebug() << "Pic type: " << type;
        const QString description=QString(buf+13+i);
        qDebug() << "Pic description: " << description;
        const int j = description.length();

        const int imgOffset = 14+i+j;
        qDebug() << "imgOffset: " << imgOffset;

        m_img = new QByteArray(buf+imgOffset, size-imgOffset);
    }
    else
        qDebug() << "Unhandled frame " << QString(buf).left(4);

    return size;
}

int tag::getExtHdrSize(const char* buf, int ver)
{
    switch (ver)
    {
    default:
    case 2:
        return 0;
    case 3:
        return getFrameSize(buf, false);
    case 4:
        return getFrameSize(buf, true);
    }
}

bool tag::checkID3v2(char* buf, bool prepend)
{
    if (!isFrame(buf, prepend ? "ID3" : "3DI"))
        return false;

    qDebug() << "ID3v2 tag found at the " << (prepend ? "beginning" : "end") << " of file";
    return true;
}

bool tag::parseID3v2header(char* buf, int& version, int& tagSize)
{
    version = (int)buf[3];
    if ((version < 2) || (version > 4))
    {
        qDebug() << "ID3v2 tag version " << QString(version).left(2) << " not supported";
        return false;
    }

    // Experimental or broken tag
    if (buf[5] & 0x3F)
    {
        qDebug() << "Experimental or broken tag";
        return false;
    }

    tagSize = getFrameSize(buf+6, true);
    qDebug() << "ID3v2 tag size: " << tagSize;

    const bool unsynch = buf[5] & 0x80;
    qDebug() << "ID3v2 unsynch: " << unsynch;
    if (unsynch)
        return false;

    //bool footer = buf[5]&0x10;

    return buf[5] & 0x40;
}

/******** APE ********/

int tag::getLE32(const char* frame)
{
    return ((unsigned int)(unsigned char)frame[0]) | ((unsigned int)(unsigned char)frame[1]<<8)
        | ((unsigned int)(unsigned char)frame[2]<<16) | ((unsigned int)(unsigned char)frame[3]<<24);
}

bool tag::getAPEItem(const char* orig, QString* dest, const char* tagName, int tagNameLen, int tagLen)
{
    if (qstricmp(orig, tagName))
        return false;

    *dest = QString::fromUtf8(orig+tagNameLen, tagLen);
    qDebug() << tagName << ": " << *dest;
    return true;
}

int tag::parseAPETag(const char* buf)
{
    int itemSize = getLE32(buf);
    //int flags = getLE32(buf+4);

    const char *ptr = buf+8;
    const int tagNameLength = qstrlen(ptr) + 1;
    qDebug() << "APE tag: " << ptr;
    if (!getAPEItem(ptr, &m_title,  "title",    tagNameLength, itemSize))
    if (!getAPEItem(ptr, &m_artist, "artist",   tagNameLength, itemSize))
    if (!getAPEItem(ptr, &m_year,   "year",     tagNameLength, itemSize))
    if (!getAPEItem(ptr, &m_album,  "album",    tagNameLength, itemSize))
    if (!getAPEItem(ptr, &m_genre,  "genre",    tagNameLength, itemSize))
    if (!getAPEItem(ptr, &m_comment, "comment", tagNameLength, itemSize))
    {
        if (!qstricmp(ptr, "track"))
        {
            m_track = QString(ptr+6);
            qDebug() << "track: " << m_track;
        }
        else if (!qstrnicmp(ptr, "Cover Art", 9))
        {
            /*
             * Cover art spec
             *
             * <length> 32 bit
             * <flags with binary bit set> 32 bit
             * <field name> "Cover Art (Front)"|"Cover Art (Back)"
             * 0x00
             * <description> UTF-8 string (needs to be a file name to be recognized by AudioShell - meh)
             * 0x00
             * <cover data> binary
             */
            const char* img = ptr + tagNameLength;
            QString desc = QString(img);
            qDebug() << "Pic description: " << desc;
            const int imgOffset = desc.length() + 1;
            qDebug() << "imgOffset: " << imgOffset;
            m_img = new QByteArray(img+imgOffset, itemSize-imgOffset);
        }
    }

    return 8 + tagNameLength + itemSize;
}

bool tag::checkAPE(char* buf, int& itemsSize, int& tagSize)
{
    if (!isFrame(buf, "APETAGEX"))
        return false;

    qDebug() << "APE tag found";

    const int version = getLE32(buf+8);
    qDebug() << "APE tag version: " << version;

    tagSize = getLE32(buf+12);
    qDebug() << "APE tag size: " << tagSize;

    qDebug() << "APE tag items: " << getLE32(buf+16);

    itemsSize = tagSize - APE_HEADER_SIZE;

    if (version > 1000)
    {
        int flags = getLE32(buf+20);
        qDebug() << "flags: " << Qt::hex << flags;

        if (flags & 0x80000000)
            tagSize += APE_HEADER_SIZE;
    }

    return true;
}
