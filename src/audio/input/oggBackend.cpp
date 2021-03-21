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

#include "oggBackend.h"

#include "oggTag.h"

#include "settings.h"
#include "utils.h"

#include <string.h>

#include <QDebug>
#include <QComboBox>
#include <QLabel>
#include <QTextCodec>

extern const unsigned char iconOgg[523] =
{
    0x47, 0x49, 0x46, 0x38, 0x39, 0x61, 0x10, 0x00, 0x0f, 0x00, 0xf6, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x27, 0x46, 0x60, 0x2e, 0x4a, 0x6e, 0x21, 0x53, 0x6d, 0x32, 0x4e, 0x74, 0x0f, 0x66, 0x6f, 0x0a,
    0x73, 0x79, 0x0a, 0x74, 0x79, 0x0f, 0x7d, 0x7c, 0x17, 0x7a, 0x7b, 0x2b, 0x7c, 0x71, 0x5a, 0x72,
    0x6f, 0x71, 0x93, 0x2c, 0x6d, 0xa2, 0x3b, 0x2d, 0x87, 0x73, 0x32, 0xa7, 0x7a, 0x70, 0x81, 0x4d,
    0x6c, 0x80, 0x5d, 0x42, 0x8a, 0x79, 0x7c, 0xda, 0x63, 0xa9, 0xab, 0x05, 0xb1, 0xae, 0x00, 0xb2,
    0xb5, 0x06, 0xae, 0xbd, 0x18, 0xb0, 0xb8, 0x1b, 0x93, 0xbd, 0x2d, 0xa7, 0xbc, 0x21, 0xc1, 0xbe,
    0x00, 0xa8, 0xc0, 0x1d, 0xb2, 0xe5, 0x35, 0xc9, 0xd0, 0x09, 0xde, 0xdc, 0x02, 0xce, 0xdd, 0x11,
    0xdd, 0xe3, 0x06, 0xd2, 0xd0, 0x2a, 0xca, 0xc9, 0x3c, 0xd5, 0xfb, 0x25, 0x87, 0x99, 0x79, 0x95,
    0xd9, 0x47, 0x3b, 0x5d, 0x85, 0x17, 0x7e, 0x8d, 0x32, 0x65, 0x83, 0x2c, 0x70, 0x89, 0x42, 0x62,
    0x8c, 0x42, 0x63, 0x8e, 0x4d, 0x6d, 0x9a, 0x48, 0x6c, 0x9d, 0x4b, 0x6f, 0x9c, 0x48, 0x6c, 0x9e,
    0x02, 0x8d, 0x8d, 0x16, 0x86, 0x91, 0x10, 0xba, 0xae, 0x01, 0xbf, 0xbf, 0x02, 0xbe, 0xbe, 0x50,
    0x8a, 0x9e, 0x74, 0x97, 0xb7, 0x78, 0x9a, 0xba, 0x2a, 0xd4, 0xb0, 0x41, 0xce, 0x93, 0x05, 0xc3,
    0xc4, 0x00, 0xc4, 0xc5, 0x0b, 0xca, 0xc3, 0x0f, 0xce, 0xc3, 0x05, 0xd1, 0xd3, 0x01, 0xe5, 0xe5,
    0x05, 0xec, 0xea, 0x00, 0xeb, 0xec, 0x02, 0xec, 0xec, 0x9e, 0x9f, 0x95, 0x87, 0x97, 0xa2, 0x8b,
    0x9d, 0xac, 0x81, 0xa7, 0xbd, 0xb2, 0xae, 0xa5, 0x8e, 0xb3, 0xcb, 0x92, 0xb6, 0xcd, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x21, 0xf9, 0x04,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x2c, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x0f, 0x00, 0x00, 0x07,
    0x68, 0x80, 0x00, 0x82, 0x83, 0x84, 0x85, 0x86, 0x82, 0x37, 0x87, 0x87, 0x4a, 0x38, 0x2d, 0x8a,
    0x84, 0x49, 0x47, 0x36, 0x2a, 0x29, 0x8f, 0x00, 0x46, 0x45, 0x12, 0x3e, 0x42, 0x3f, 0x32, 0x8a,
    0x25, 0x44, 0x48, 0x18, 0x19, 0x3a, 0x41, 0x34, 0x07, 0x31, 0x85, 0x0a, 0x1a, 0x22, 0x23, 0x1f,
    0x1b, 0x16, 0x0d, 0x33, 0x40, 0x3b, 0x85, 0x28, 0x0f, 0x21, 0x24, 0x13, 0x1e, 0x15, 0x14, 0x39,
    0x3c, 0x05, 0x86, 0x09, 0x1c, 0x1d, 0x26, 0x20, 0x1c, 0x0c, 0x3d, 0x43, 0x34, 0x87, 0x0b, 0x0b,
    0x10, 0x0e, 0x09, 0x01, 0x03, 0x06, 0x8f, 0x2d, 0x2f, 0x27, 0x02, 0x04, 0x96, 0x82, 0x2f, 0x2b,
    0xdb, 0xdc, 0x82, 0x2b, 0xe1, 0xe4, 0xe5, 0x00, 0x81, 0x00, 0x3b
};

ov_callbacks oggBackend::vorbis_callbacks =
{
    oggBackend::read_func,
    oggBackend::seek_func,
    oggBackend::close_func,
    oggBackend::tell_func
};

#define EXT "ogg|oga"

#define CREDITS "OggVorbis<br>Copyright \302\251 Xiph.org Foundation"
#define LINK    "http://www.xiph.org/"

const char oggBackend::name[] = "Ogg-Vorbis";

oggConfig_t oggConfig::m_settings;

inputConfig* oggBackend::cFactory() { return new oggConfig(name, iconOgg, 523); }

/*****************************************************************/

size_t oggBackend::fillBuffer(void* buffer, const size_t bufferSize)
{
    size_t n = 0;
    long read;
    do {
        int current_section;
        read = ov_read(m_vf, (char*)buffer+n, bufferSize-n, 0,
                (m_config.precision() == sample_t::S16) ? 2 : 1,
                (m_config.precision() != sample_t::U8), &current_section);
        n += read;
    } while (read && (n < bufferSize));

    return n;
}

/*****************************************************************/

void oggConfig::loadSettings()
{
    m_settings.precision = (load("Bits", 16) == 16) ? sample_t::S16 : sample_t::U8;
}

void oggConfig::saveSettings()
{
    save("Bits", (m_settings.precision == sample_t::S16) ? 16 : 8);
}

/*****************************************************************/

QStringList oggBackend::ext() { return QString(EXT).split("|"); }

oggBackend::oggBackend() :
    m_vf(nullptr),
    m_vi(nullptr),
    m_config(name, iconOgg, 523)
{}

oggBackend::~oggBackend()
{
    if (m_vf != nullptr)
    {
        ov_clear(m_vf);
        m_file.close();
    }
}

bool oggBackend::open(const QString& fileName)
{
    m_file.setFileName(fileName);
    if (!m_file.open(QIODevice::ReadOnly))
    {
        qWarning() << m_file.errorString();
        return false;
    }

    m_vf = new OggVorbis_File;
    int error = ov_open_callbacks(&m_file, m_vf, NULL, 0, vorbis_callbacks);
    if (error < 0)
    {
        qDebug() << "Error code: " << error;
        utils::delPtr(m_vf);
        m_file.close();
        return false;
    }

    m_vi = ov_info(m_vf, -1);

    setDuration(static_cast<unsigned int>(ov_time_total(m_vf, -1)*1000.));

    QString title;
    QString artist;
    QString year;
    QString album;
    QString genre;
    QString comment;
    QString lyrics;
    QString mime;
    QByteArray image;

    char **ptr = ov_comment(m_vf, -1)->user_comments;
    while (*ptr)
    {
        qDebug() << *ptr;
        if (!oggTag::getMetadata(*ptr, &title, "title"))
        if (!oggTag::getMetadata(*ptr, &artist, "artist"))
        if (!oggTag::getMetadata(*ptr, &year, "date"))
        if (!oggTag::getMetadata(*ptr, &album, "album"))
        if (!oggTag::getMetadata(*ptr, &genre, "genre"))
        if (!oggTag::getMetadata(*ptr, &comment, "comment"))
        {
            if (oggTag::isTag(*ptr, "tracknumber"))
            {
                m_metaData.addInfo(metaData::TRACK, QString(*ptr).mid(12));
            }
            else if (oggTag::isTag(*ptr, "UNSYNCEDLYRICS"))
            {
                lyrics = QString(*ptr+15);
            }
            else if (oggTag::isTag(*ptr, "METADATA_BLOCK_PICTURE"))
            {
                oggTag::readBlockPicture(QByteArray::fromBase64(*ptr+23), image, mime);
            }
            else if (oggTag::isTag(*ptr, "COVERARTMIME"))
            {
                mime = QString(*ptr+13);
            }
            else if (oggTag::isTag(*ptr, "COVERART"))
            {
                image = QByteArray::fromBase64(*ptr+9);
            }
            else if (oggTag::isTag(*ptr, "BINARY_COVERART"))
            {
                // like METADATA_BLOCK_PICTURE but not encoded
                quint32 picType = oggTag::getNum(*ptr+16);
                qDebug() << "picType: " << picType;
                quint32 mimeLen = oggTag::getNum(*ptr+20);
                mime = QString(QByteArray::fromRawData(*ptr+24, mimeLen));
                qDebug() << "mime: " << mime;
                quint32 descLen = oggTag::getNum(*ptr+24+mimeLen);
                QString desc = QString(QByteArray::fromRawData(*ptr+28+mimeLen, descLen));
                qDebug() << "desc: " << desc;

                // FIXME - WTF! the image is UTF8 encoded!
                const quint32 dataPos = 28+mimeLen+descLen+16;
                quint32 dataLen = oggTag::getNum(*ptr+dataPos);
#if 0
                QString imgData = QString::fromUtf8(*ptr+4+dataPos, dataLen);
                for (auto i = imgData.constBegin(); i != imgData.constEnd(); ++i)
                {
                    image.append(static_cast<char >((*i).unicode()));
                }
#else
                QTextCodec *codec = QTextCodec::codecForName("UTF-8");
                QTextDecoder *decoder = new QTextDecoder(codec, QTextCodec::ConvertInvalidToNull);

                for (int i = 0; i < dataLen; i++)
                {
                    QString imgData = decoder->toUnicode(*ptr+4+dataPos+i, 1);
                    if (!imgData.isEmpty())
                    {
                        ushort unicode = imgData.front().unicode();
                        if (unicode>255)
                            qWarning() << "out of range: " << unicode;
                        image.append(static_cast<char>(imgData.front().unicode()));
                    }
                }
                delete decoder;
            }
#endif
        }
        ++ptr;
    }

    m_metaData.addInfo(metaData::TITLE, title);
    m_metaData.addInfo(metaData::ARTIST, artist);
    m_metaData.addInfo(metaData::ALBUM, album);
    m_metaData.addInfo(metaData::GENRE, genre);
    m_metaData.addInfo(metaData::YEAR, year);
    m_metaData.addInfo(metaData::COMMENT, comment);
    m_metaData.addInfo(metaData::LYRICS, lyrics);

    if (!mime.isNull())
        m_metaData.addInfo(new QByteArray(image));

    songLoaded(fileName);
    return true;
}

bool oggBackend::seek(int pos)
{
    if (m_vf == nullptr)
        return false;

    ogg_int64_t length = ov_pcm_total(m_vf, -1);

    if (length < 0)
        return false;

    ogg_int64_t offset = (length * pos) / 100;
    if (ov_pcm_seek(m_vf, offset) != 0)
        return false;

    return true;
}

size_t oggBackend::read_func(void *ptr, size_t size, size_t nmemb, void *datasource)
{
    return ((QFile*)datasource)->read((char*)ptr, size*nmemb);
}

int oggBackend::seek_func(void *datasource, ogg_int64_t offset, int whence)
{
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

#define OGGSETTINGS	oggConfig::m_settings

oggConfigFrame::oggConfigFrame(QWidget* win) :
    configFrame(win, oggBackend::name, CREDITS, LINK)
{
    matrix()->addWidget(new QLabel(tr("Bits"), this), 0, 0);
    QComboBox* _bitBox = new QComboBox(this);
    matrix()->addWidget(_bitBox, 0, 1);

    QStringList items;
    items << "8" << "16";
    _bitBox->addItems(items);
    _bitBox->setMaxVisibleItems(items.size());

    int val;
    switch (OGGSETTINGS.precision)
    {
    case sample_t::U8:
        val = 0;
        break;
    default:
    case sample_t::S16:
        val = 1;
        break;
    }
    _bitBox->setCurrentIndex(val);
    connect(_bitBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
        [](int val) {
            switch (val)
            {
            case 0:
                OGGSETTINGS.precision = sample_t::U8;
                break;
            case 1:
                OGGSETTINGS.precision = sample_t::S16;
                break;
            }
        }
    );
}
