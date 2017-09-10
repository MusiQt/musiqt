/*
 *  Copyright (C) 2006-2017 Leandro Nini
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

#include "opusBackend.h"

#include "settings.h"
#include "utils.h"

#include <string.h>

#include <QDebug>
#include <QComboBox>
#include <QLabel>

OpusFileCallbacks opusBackend::opus_callbacks =
{
    opusBackend::read_func,
    opusBackend::seek_func,
    opusBackend::tell_func,
    opusBackend::close_func
};

#define EXT "opus"

#define CREDITS "Opus\nCopyright \302\251 Xiph.org Foundation"
#define LINK    "http://opus-codec.org/"

const char opusBackend::name[] = "Opus";

opusConfig_t opusBackend::_settings;

/*****************************************************************/

size_t opusBackend::fillBuffer(void* buffer, const size_t bufferSize, const unsigned int seconds)
{
    size_t n = 0;
    int read;
    do {
        read = op_read_stereo(_of, (opus_int16*)(buffer+n), (bufferSize-n)/2);
        if (read < 0)
        {
            qWarning() << "Decoding error: " << read;
            return 0;
        }
        n += read*4;
    } while (read && (n < bufferSize));

    return n;
}

/*****************************************************************/

bool opusBackend::supports(const QString& fileName)
{
    QRegExp rx("*." EXT);
    rx.setPatternSyntax(QRegExp::Wildcard);
    return rx.exactMatch(fileName);
}

inline QStringList opusBackend::ext() const { return QStringList(EXT); }

opusBackend::opusBackend() :
    inputBackend(name),
    _of(nullptr)
{}

opusBackend::~opusBackend()
{
    close();
}

bool opusBackend::compareTag(const char* orig, const char* tag)
{
    int n = qstrlen(tag);
    return qstrnicmp(orig, tag, n);
}

bool opusBackend::open(const QString& fileName)
{
    close();

    _file.setFileName(fileName);
    if (!_file.open(QIODevice::ReadOnly))
        return false;

    int error;
    _of = op_open_callbacks(&_file, &opus_callbacks, nullptr, 0, &error);
    if (_of == nullptr)
    {
        qDebug() << "Error code: " << error;
        _file.close();
        return false;
    }

    time((int)(op_pcm_total(_of, -1)/48000));

    QString title;
    QString artist;
    QString year;
    QString album;
    QString genre;
    QString comment;
    QString mime;
    QByteArray image;

    char **ptr = op_tags(_of, -1)->user_comments;
    while (*ptr)
    {
        qDebug() << *ptr;
        if (!getComment(*ptr, &title, "title"))
        if (!getComment(*ptr, &artist, "artist"))
        if (!getComment(*ptr, &year, "date"))
        if (!getComment(*ptr, &album, "album"))
        if (!getComment(*ptr, &genre, "genre"))
        if (!getComment(*ptr, &comment, "comment"))
        {
            if (!compareTag(*ptr, "tracknumber"))
            {
                _metaData.addInfo(metaData::TRACK, QString(*ptr).mid(12));
            }
            else if (!compareTag(*ptr, "METADATA_BLOCK_PICTURE"))
            {
                qDebug() << "METADATA_BLOCK_PICTURE";
                QByteArray flac_picture = QByteArray::fromBase64(*ptr+23);
            }
            else if (!compareTag(*ptr, "COVERARTMIME"))
            {
                qDebug() << "COVERARTMIME";
                mime = QString(*ptr+13);
            }
            else if (!compareTag(*ptr, "COVERART"))
            {
                qDebug() << "COVERART";
                image = QByteArray::fromBase64(*ptr+9);
            }
            else if (!compareTag(*ptr, "BINARY_COVERART"))
            {
                qDebug() << "BINARY_COVERART";
                QByteArray flac_picture(*ptr+16); // FIXME
            }
        }
        ++ptr;
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

//deprecated:
//COVERARTMIME
//COVERART (base64 encoded)

    _metaData.addInfo(metaData::TITLE, title);
    _metaData.addInfo(metaData::ARTIST, artist);
    _metaData.addInfo(metaData::ALBUM, album);
    _metaData.addInfo(metaData::GENRE, genre);
    _metaData.addInfo(metaData::YEAR, year);
    _metaData.addInfo(metaData::COMMENT, comment);

    if (!mime.isNull())
        _metaData.addInfo(new imageData(image.size(), (char*)image.data(), mime));

    songLoaded(fileName);
    return true;
}

bool opusBackend::getComment(const char* orig, QString* dest, const char* type)
{
    const int n = qstrlen(type);
    if (qstrnicmp(orig, type, n))
        return false;

    if (orig[n] != '=')
        return false;

    if (!dest->isEmpty())
        dest->append(", ");

    dest->append(QString::fromUtf8(orig+n+1));
    return true;
}

void opusBackend::close()
{
    if (_of != nullptr)
    {
        op_free(_of);
        //utils::delPtr(_of);
        _file.close();
    }

    songLoaded(QString::null);
}

bool opusBackend::rewind()
{
    if (_of == nullptr)
        return false;

    if (op_raw_seek(_of, 0))
        return false;

    return true;
}

int opusBackend::read_func(void *_stream, unsigned char *_ptr, int _nbytes)
{
    return ((QFile*)_stream)->read((char*)_ptr, _nbytes);
}

int opusBackend::seek_func(void *_stream, opus_int64 _offset, int _whence)
{
    QFile* file = static_cast<QFile*>(_stream);
    qint64 pos;
    switch (_whence)
    {
    case SEEK_SET:
        pos = 0;
        break;
    case SEEK_CUR:
        pos = file->pos();
        break;
    case SEEK_END:
        pos = file->size();
        break;
    default:
        return -1;
    }
    return file->seek(pos + _offset) ? 0 : -1;
}

opus_int64 opusBackend::tell_func(void *_stream)
{
    return ((QFile*)_stream)->pos();
}

int opusBackend::close_func(void *_stream)
{
    ((QFile*)_stream)->close();
    return 0;
}

/*****************************************************************/

opusConfig::opusConfig(QWidget* win) :
    configFrame(win, opusBackend::name, CREDITS, LINK)
{
    matrix()->addWidget(new QLabel(tr("No settings available"), this));
}
