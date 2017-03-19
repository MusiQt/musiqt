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

#include "oggBackend.h"

#include "settings.h"
#include "utils.h"

#include <string.h>

#include <QDebug>
#include <QComboBox>
#include <QLabel>

ov_callbacks oggBackend::vorbis_callbacks =
{
    oggBackend::read_func,
    oggBackend::seek_func,
    oggBackend::close_func,
    oggBackend::tell_func
};

#define EXT "ogg"

#define CREDITS	 "OggVorbis\nCopyright \302\251 Xiph.org Foundation"
#define LINK     "http://www.xiph.org/"

const char oggBackend::name[]="Ogg-Vorbis";

oggConfig_t oggBackend::_settings;

/*****************************************************************/

size_t oggBackend::fillBuffer(void* buffer, const size_t bufferSize, const unsigned int seconds)
{
    size_t n=0;
    long read;
    do {
        int current_section;
        read=ov_read(_vf, (char*)buffer+n, bufferSize-n, 0,
                (_settings.precision==S16)?2:1, (_settings.precision!=U8), &current_section);
        n+=read;
    } while (read && n<bufferSize);

    return n;
}

/*****************************************************************/

bool oggBackend::supports(const QString& fileName)
{
    QRegExp rx("*." EXT);
    rx.setPatternSyntax(QRegExp::Wildcard);
    return rx.exactMatch(fileName);
}

inline QStringList oggBackend::ext() const { return QStringList(EXT); }

oggBackend::oggBackend() :
    inputBackend(name),
    _vf(nullptr),
    _vi(nullptr)
{
    loadSettings();
}

oggBackend::~oggBackend()
{
    close();
}

void oggBackend::loadSettings()
{
    _settings.precision=(load("Bits", 16)==16)?S16:U8;
}

void oggBackend::saveSettings()
{
    save("Bits", (_settings.precision==S16)?16:8);
}

bool oggBackend::open(const QString& fileName)
{
    close();

    _file.setFileName(fileName);
    if (!_file.open(QIODevice::ReadOnly))
        return false;

    _vf=new OggVorbis_File;
    if (ov_open_callbacks(&_file, _vf, NULL, 0, vorbis_callbacks)<0)
    {
        delPtr(_vf);
        _file.close();
        return false;
    }

    _vi=ov_info(_vf, -1);

    time((int)ov_time_total(_vf, -1));

    QString title = QString::null;
    QString artist = QString::null;
    QString year = QString::null;
    QString album = QString::null;
    QString genre = QString::null;
    QString comment = QString::null;

    char **ptr=ov_comment(_vf,-1)->user_comments;
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
            if (!QString::compare(*ptr, "tracknumber"))
                _metaData.addInfo(metaData::TRACK, QString(*ptr).mid(12));
        }
        ++ptr;
    }

//METADATA_BLOCK_PICTURE
/*
<32> 	 The picture type according to the ID3v2 APIC frame:
<32> 	 The length of the MIME type string in bytes.
<n*8> 	 The MIME type string
<32> 	 The length of the description string in bytes.
<n*8> 	 The description of the picture, in UTF-8. 
 <32> 	 The width of the picture in pixels. 
 <32> 	 The height of the picture in pixels. 
 <32> 	 The color depth of the picture in bits-per-pixel. 
 <32> 	 For indexed-color pictures (e.g. GIF), the number of colors used, or 0 for non-indexed pictures. 
 <32> 	 The length of the picture data in bytes. 
 <n*8> 	 The binary picture data.
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

    songLoaded(fileName);
    return true;
}

bool oggBackend::getComment(const char* orig, QString* dest, const char* type)
{
    if (QString::compare(orig, type))
        return false;

    if (!dest->isEmpty())
        dest->append(", ");

    const int n = strlen(type);
    dest->append(QString::fromUtf8(orig+n+1));
    return true;
}

void oggBackend::close()
{
    if (_vf != nullptr)
    {
        ov_clear(_vf);
        delPtr(_vf);
        _file.close();
    }

    _vi = nullptr;

    songLoaded(QString::null);
}

bool oggBackend::rewind()
{
    if (_vf == nullptr)
        return false;

    if (ov_raw_seek(_vf, 0))
        return false;

    return true;
}

size_t oggBackend::read_func(void *ptr, size_t size, size_t nmemb, void *datasource)
{
    return ((QFile*)datasource)->read((char*)ptr, size*nmemb);
}

int oggBackend::seek_func(void *datasource, ogg_int64_t offset, int whence) {

    QFile* file = static_cast<QFile*>(datasource);
    qint64 pos;
    switch (whence)
    {
    case SEEK_SET:
            pos = offset;
            break;
    case SEEK_CUR:
            pos = file->pos() + offset;
            break;
    case SEEK_END:
            pos = file->size() + offset;
            break;
    default:
            return -1;
    }
    return file->seek(pos);
}

int oggBackend::close_func(void *datasource)
{
    ((QFile*)datasource)->close();
    return 0;
}

long oggBackend::tell_func(void *datasource)
{
    return ((QFile*)datasource)->pos();
}

/*****************************************************************/

#define OGGSETTINGS	oggBackend::_settings

oggConfig::oggConfig(QWidget* win) :
    configFrame(win, oggBackend::name, CREDITS, LINK)
{
    matrix()->addWidget(new QLabel(tr("Bits"), this));
    QComboBox* _bitBox = new QComboBox(this);
    matrix()->addWidget(_bitBox);
    
    QStringList items;
    items << "8" << "16";
    _bitBox->addItems(items);
    _bitBox->setMaxVisibleItems(2);
    
    int val;
    switch (OGGSETTINGS.precision)
    {
    case U8:
        val=0;
        break;
    default:
    case S16:
        val=1;
        break;
    }
    _bitBox->setCurrentIndex(val);
    connect(_bitBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onCmdBits(int)));
}

void oggConfig::onCmdBits(int val)
{
    switch (val)
    {
    case 0:
        OGGSETTINGS.precision = U8;
        break;
    case 1:
        OGGSETTINGS.precision = S16;
        break;
    }
}
