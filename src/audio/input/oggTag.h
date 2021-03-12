/*
 *  Copyright (C) 2006-2021 Leandro Nini
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

#ifndef OGGTAG_H
#define OGGTAG_H

#include <QDebug>
#include <QByteArray>
#include <QString>

namespace oggTag
{
    static bool isTag(const char* orig, const char* tagName)
    {
        const int n = qstrlen(tagName);
        if (qstrnicmp(orig, tagName, n))
            return false;

        if (orig[n] != '=')
            return false;

        return true;
    }

    static quint32 getNum(const char* orig)
    {
        // big-endian
        quint32 res = 0;
        res |= static_cast<unsigned char>(*orig++) << 24;
        res |= static_cast<unsigned char>(*orig++) << 16;
        res |= static_cast<unsigned char>(*orig++) << 8;
        res |= static_cast<unsigned char>(*orig);
        return res;
    }

    static bool getMetadata(const char* orig, QString* dest, const char* tagName)
    {
        if (isTag(orig, tagName))
        {
            if (!dest->isEmpty())
                dest->append(", ");

            const int n = qstrlen(tagName);
            dest->append(QString::fromUtf8(orig+n+1));
            return true;
        }

        return false;
    }

    //METADATA_BLOCK_PICTURE
    /*
    <32>   The picture type according to the ID3v2 APIC frame:
    <32>   The length of the MIME type string in bytes.
    <n*8>  The MIME type string
    <32>   The length of the description string in bytes.
    <n*8>  The description of the picture, in UTF-8.
    <32>   The width of the picture in pixels.
    <32>   The height of the picture in pixels.
    <32>   The color depth of the picture in bits-per-pixel.
    <32>   For indexed-color pictures (e.g. GIF), the number of colors used, or 0 for non-indexed pictures.
    <32>   The length of the picture data in bytes.
    <n*8>  The binary picture data.
    */
    static void readBlockPicture(const QByteArray &data, QByteArray& image, QString& mime)
    {
        const char* ptr = data.constData();
        quint32 picType = getNum(ptr);
        qDebug() << "picType: " << picType;
        quint32 mimeLen = getNum(ptr+4);
        mime = QString(QByteArray::fromRawData(ptr+8, mimeLen));
        qDebug() << "mime: " << mime;
        quint32 descLen = getNum(ptr+8+mimeLen);
        QString desc = QString(QByteArray::fromRawData(ptr+8+mimeLen, descLen));
        qDebug() << "desc: " << desc;

        const quint32 dataPos = 8+mimeLen+4+descLen+16;
        quint32 dataLen = getNum(ptr+dataPos);

        image = QByteArray(data.right(dataLen));
    }
};

#endif
