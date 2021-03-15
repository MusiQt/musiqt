/*
 *  Copyright (C) 2009-2021 Leandro Nini
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

#include "mpg123Backend.h"

#include "settings.h"
#include "genres.h"

#include <algorithm>

#include <QDebug>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>

extern const unsigned char iconMpg123[560] =
{
    0x47,0x49,0x46,0x38,0x39,0x61,0x10,0x00,0x10,0x00,0xf6,0x00,0x00,0xef,0x29,0x29,
    0xef,0x2e,0x2e,0xf0,0x30,0x30,0xf0,0x31,0x31,0xf0,0x35,0x35,0xf0,0x36,0x36,0xf1,
    0x3b,0x3b,0xf0,0x3c,0x3c,0xf0,0x40,0x40,0xf1,0x4d,0x4d,0xf3,0x51,0x51,0xf2,0x56,
    0x56,0xf2,0x59,0x59,0xf2,0x5b,0x5b,0xf1,0x5d,0x5d,0xf2,0x62,0x62,0xf3,0x62,0x62,
    0xf3,0x66,0x66,0xf5,0x6d,0x6d,0xf5,0x76,0x76,0xf5,0x77,0x77,0x73,0xd2,0x16,0x76,
    0xd3,0x1b,0x77,0xd2,0x1d,0x78,0xd3,0x1e,0x7c,0xd5,0x26,0x7e,0xd5,0x2c,0x80,0xd2,
    0x2e,0x8c,0xd7,0x42,0x8a,0xd2,0x44,0x8b,0xd9,0x41,0x8f,0xd9,0x44,0x92,0xc7,0x5d,
    0x94,0xcd,0x5c,0x9c,0xc8,0x71,0x9f,0xc0,0x7f,0xa1,0xd8,0x6c,0xab,0xe4,0x71,0x20,
    0x4a,0x87,0x26,0x4f,0x8a,0x55,0x75,0xa4,0x6b,0x81,0x9d,0x72,0x8d,0xb2,0x87,0x8a,
    0x84,0x88,0x8b,0x86,0x8c,0x8e,0x89,0x8d,0x8f,0x8a,0x8d,0x90,0x8a,0x8f,0x94,0x89,
    0x8e,0x96,0x9d,0x92,0x95,0x91,0x96,0x98,0x94,0x92,0x97,0x99,0x97,0xa7,0x88,0x9e,
    0xbd,0x80,0x9f,0xbc,0x82,0x9f,0xb8,0x87,0x9f,0xa0,0x9b,0x9e,0xa2,0x9e,0xa1,0xba,
    0x87,0xa5,0xbf,0x8d,0x87,0x95,0xa5,0x89,0x96,0xa6,0x83,0x96,0xac,0x89,0x97,0xab,
    0x89,0x98,0xab,0x89,0x99,0xae,0x82,0x98,0xb7,0x80,0x98,0xb9,0x87,0x9e,0xbf,0x97,
    0xa0,0xac,0x9b,0xa4,0xab,0x9b,0xa5,0xaf,0x93,0xa2,0xb6,0xb7,0xb9,0xb5,0xee,0x8a,
    0x8a,0xf6,0x82,0x82,0xf3,0x84,0x84,0xb0,0xd9,0x86,0xb4,0xd8,0x8e,0xb4,0xcf,0x9b,
    0xb7,0xe6,0x85,0xb8,0xe0,0x91,0xb3,0xc0,0xa7,0xbd,0xd5,0xa5,0xaf,0xbc,0xce,0xd1,
    0xd3,0xd3,0xd3,0xd5,0xd3,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x21,0xf9,0x04,
    0x01,0x00,0x00,0x58,0x00,0x2c,0x00,0x00,0x00,0x00,0x10,0x00,0x10,0x00,0x00,0x07,
    0x8d,0x80,0x58,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x85,0x55,0x56,0x4a,0x8a,
    0x58,0x2a,0x27,0x26,0x47,0x3a,0x50,0x18,0x15,0x16,0x25,0x58,0x12,0x00,0x00,0x06,
    0x58,0x45,0x43,0x26,0x31,0x32,0x3b,0x4e,0x54,0x1c,0x1f,0x84,0x03,0x0f,0x58,0x40,
    0xa3,0x2b,0x2e,0x2d,0x2f,0x20,0x1e,0x84,0x07,0x11,0x58,0x41,0xb0,0x2b,0x2b,0x30,
    0x1d,0x21,0x82,0x4c,0x0a,0x10,0x82,0x3d,0xbc,0x2b,0x22,0x1a,0x35,0x33,0x58,0x4b,
    0x0e,0x01,0x4d,0x58,0x3e,0xbc,0x37,0x17,0x23,0x2b,0x2c,0x39,0x82,0x09,0x0c,0x58,
    0x49,0x26,0x48,0x34,0x35,0x1b,0x24,0x38,0x3c,0x84,0x08,0x0d,0x28,0x26,0xed,0x29,
    0x36,0x15,0xf1,0x19,0x58,0x0b,0x05,0x04,0x02,0x82,0x44,0x3f,0x42,0x46,0x53,0x51,
    0x4f,0x52,0x06,0x51,0x98,0x70,0xe8,0xca,0xa3,0x83,0x08,0x0d,0x05,0x02,0x00,0x3b
};

// mp3|mp2
#define EXT "mp3"

#define CREDITS "MPEG Audio Decoder library<br>Copyright \302\251 the mpg123 project"
#define LINK    "http://mpg123.org/"

const char mpg123Backend::name[] = "Mpg123";

int mpg123Backend::m_status = 0;

QStringList mpg123Backend::m_decoders;

mpg123Config_t mpg123Backend::m_settings;

/*****************************************************************/

size_t mpg123Backend::fillBuffer(void* buffer, const size_t bufferSize)
{
    size_t n;
    const int err = mpg123_read(m_handle, (unsigned char*)buffer, bufferSize, &n);
    if (err != MPG123_OK)
    {
        qWarning() << mpg123_plain_strerror(err);
        return 0;
    }

    return n;
}

/*****************************************************************/

QStringList mpg123Backend::ext() { return QStringList(EXT); }

mpg123Backend::mpg123Backend() :
    inputBackend(name, iconMpg123, 560),
    m_handle(nullptr)
{
    loadSettings();

    if (++m_status > 1)
        return;

    const int err = mpg123_init();
    if (err == MPG123_OK)
    {
        m_decoders << "auto";
        const char** decoders = mpg123_supported_decoders();
        while (*decoders)
        {
            qDebug() << "Decoder: " << *decoders;
            m_decoders << *decoders;
            ++decoders;
        }
    }
    else
    {
        qWarning() << mpg123_plain_strerror(err);
        m_status = 0;
    }
}

mpg123Backend::~mpg123Backend()
{
    close();

    if (--m_status == 0)
        mpg123_exit();
}

void mpg123Backend::loadSettings()
{
    m_settings.fastscan = load("Fastscan", true);
    m_settings.decoder = load("Decoder", "auto");
}

void mpg123Backend::saveSettings()
{
    save("Fastscan", m_settings.fastscan);
    save("Decoder", m_settings.decoder);
}

bool mpg123Backend::open(const QString& fileName)
{
    if (!m_status)
        return false;

    close();

    m_file.setFileName(fileName);
    if (!m_file.open(QIODevice::ReadOnly))
    {
        qWarning() << m_file.errorString();
        return false;
    }

    int err;
    qDebug() << "Setting decoder: " << m_settings.decoder;
    const char *decoder = QString::compare(m_settings.decoder, "auto")
        ? m_settings.decoder.toLocal8Bit().constData()
        : nullptr;
    m_handle = mpg123_new(decoder, &err);
    if (m_handle == nullptr)
        goto error;

    err = mpg123_replace_reader_handle(m_handle, read_func, seek_func, NULL);
    if (err == MPG123_OK)
    {
        err = mpg123_open_handle(m_handle, &m_file);
    }
    if (err != MPG123_OK)
    {
        mpg123_delete(m_handle);
        goto error;
    }

    if (!m_settings.fastscan)
    {
        err = mpg123_param(m_handle, MPG123_ADD_FLAGS, MPG123_PICTURE, 0.);

        err = mpg123_scan(m_handle);
        if (err != MPG123_OK)
        {
            qWarning() << mpg123_plain_strerror(err);
        }
    }

    int encoding;
    err = mpg123_getformat(m_handle, &m_samplerate, &m_channels, &encoding);
    if (err != MPG123_OK)
    {
        mpg123_delete(m_handle);
        goto error;
    }

    err = mpg123_length(m_handle);
    if (err != MPG123_ERR)
    {
        time((err*1000LL)/m_samplerate);
    }

    err = mpg123_param(m_handle, MPG123_RVA,
        SETTINGS->replayGain()
            ? (SETTINGS->replayGainMode() == 0) ? MPG123_RVA_ALBUM : MPG123_RVA_MIX
            : MPG123_RVA_OFF,
        0.);

    mpg123_id3v1* id3v1;
    mpg123_id3v2* id3v2;
    err = mpg123_id3(m_handle, &id3v1, &id3v2);
    if (err == MPG123_OK)
    {
        QString info;

        if (id3v2 && id3v2->title)
        {
            info = QString::fromUtf8(id3v2->title->p);
        }
        else if (id3v1)
        {
            info = QString(id3v1->title).trimmed();
            if (info.size() > 30)
                info.resize(30);
        }
        else
        {
            info = QString();
        }
        qDebug() << "TITLE: " << info;
        m_metaData.addInfo(metaData::TITLE, info);

        if (id3v2 && id3v2->artist)
        {
            info = QString::fromUtf8(id3v2->artist->p);
        }
        else if (id3v1)
        {
            info = QString(id3v1->artist).trimmed();
            if (info.size() > 30)
                info.resize(30);
        }
        else
        {
            info = QString();
        }
        qDebug() << "ARTIST: " << info;
        m_metaData.addInfo(metaData::ARTIST, info);

        if (id3v2 && id3v2->album)
        {
            info = QString::fromUtf8(id3v2->album->p);
        }
        else if (id3v1)
        {
            info = QString(id3v1->album).trimmed();
            if (info.size() > 30)
                info.resize(30);
        }
        else
        {
            info = QString();
        }
        qDebug() << "ALBUM: " << info;
        m_metaData.addInfo(metaData::ALBUM, info);

        if (id3v2 && id3v2->genre)
        {
            qDebug() << "genre id3v2: " << id3v2->genre->p;
            info = QString::fromUtf8(id3v2->genre->p);
            int st = info.indexOf('(');
            if (st >= 0)
            {
                QStringRef tmp = info.midRef(st, info.indexOf(')')-st);
                const unsigned int idx = tmp.string()->toInt();
                if (idx < GENRES)
                    info = QString(genre[idx]);
            }
        }
        else if (id3v1 && (id3v1->genre < GENRES))
        {
            info = genre[id3v1->genre];
        qDebug() << "genre id3v1: " << id3v1->genre;
        }
        else
        {
            info = QString();
        }
        qDebug() << "GENRE: " << info;
        m_metaData.addInfo(metaData::GENRE, info);

        if (id3v2 && id3v2->year)
        {
            info = QString::fromUtf8(id3v2->year->p);
            // TODO support multiple genres
            // RX    Remix
            // CR    Cover
        }
        else if (id3v1)
        {
            info = QString(id3v1->year).trimmed();
            if (info.size() > 4)
                info.resize(4);
        }
        else
        {
            info = QString();
        }
        qDebug() << "YEAR: " << info;
        m_metaData.addInfo(metaData::YEAR, info);

        if (id3v1 && (id3v1->comment[28] == 0))
        {
            info = QString::number(id3v1->comment[29]);
        }
        else
        {
            info = QString();
        }
        qDebug() << "TRACK: " << info;
        m_metaData.addInfo(metaData::TRACK, info);

        info = QString();
        if (id3v2)
        {
            for (unsigned int i=0; i<id3v2->comments; i++)
            {
                mpg123_text *entry = &id3v2->comment_list[i];
                if ((entry->description.fill == 0) || (entry->description.p[0] == 0))
                    info = QString::fromUtf8(entry->text.p).trimmed();
            }
        }

        if (id3v2 && id3v2->pictures)
        {
            mpg123_picture picture = id3v2->picture[0];
            QString mime(picture.mime_type.p);
            qDebug() << "mime: " << mime;
            QString desc(picture.description.p);
            qDebug() << "description: " << desc;
            m_metaData.addInfo(new QByteArray((char*)picture.data, picture.size));
        }

        if (info.isEmpty() && id3v1)
        {
            info = QString(id3v1->comment).trimmed();
            if (info.size() > 30)
                info.resize(30);
        }

        if (!info.isEmpty())
        {
            qDebug() << "COMMENT: " << info;
            m_metaData.addInfo(metaData::COMMENT, info);
        }
    }

    songLoaded(fileName);
    return true;

error:
    qWarning() << mpg123_strerror(m_handle);
    m_file.close();
    return false;
}

void mpg123Backend::close()
{
    if (songLoaded().isEmpty())
        return;

    mpg123_close(m_handle);
    mpg123_delete(m_handle);

    m_file.close();

    songLoaded(QString());
}

bool mpg123Backend::seek(int pos)
{
    if (songLoaded().isEmpty())
        return false;

    off_t frames = mpg123_length(m_handle);
    if (frames < 0)
        return false;

    off_t offset = (frames * pos) / 100;
    if (mpg123_seek(m_handle, offset, SEEK_SET) < 0)
    {
        qWarning() << mpg123_strerror(m_handle);
        return false;
    }

    return true;
}

ssize_t mpg123Backend::read_func(void* handle, void* ptr, size_t size)
{
    return ((QFile*)handle)->read((char*)ptr, size);
}

off_t mpg123Backend::seek_func(void* handle, off_t offset, int whence)
{
    qint64 pos;
    switch (whence)
    {
    case SEEK_SET:
        pos = offset;
        break;
    case SEEK_CUR:
        pos = ((QFile*)handle)->pos() + offset;
        break;
    case SEEK_END:
        pos = ((QFile*)handle)->size() + offset;
        break;
    default:
        return -1;
    }
    return ((QFile*)handle)->seek(pos) ? pos : -1;
}

/*****************************************************************/

#define MPGSETTINGS	mpg123Backend::m_settings

mpg123Config::mpg123Config(QWidget* win) :
    configFrame(win, mpg123Backend::name, CREDITS, LINK)
{
    matrix()->addWidget(new QLabel(tr("Decoder"), this), 0, 0);
    QComboBox *decBox = new QComboBox(this);
    matrix()->addWidget(decBox, 0, 1);
    decBox->addItems(mpg123Backend::m_decoders);
    const int numItems = decBox->count();
    decBox->setMaxVisibleItems(std::min(numItems, 5));
    const int curItem = decBox->findText(MPGSETTINGS.decoder);
    if (curItem >= 0)
        decBox->setCurrentIndex(curItem);
    connect(decBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
        [](int val) {
            MPGSETTINGS.decoder = mpg123Backend::m_decoders.at(val);
        }
    );

    {
        QFrame* line = new QFrame();
        line->setFrameShape(QFrame::VLine);
        line->setFrameShadow(QFrame::Sunken);
        extraLeft()->addWidget(line);
    }

    QVBoxLayout *vert = new QVBoxLayout();
    extraLeft()->addLayout(vert);
    QCheckBox *cBox = new QCheckBox(tr("Fast scan"));
    cBox->setChecked(MPGSETTINGS.fastscan);
    cBox->setToolTip("Scan only few frame of file,\ntime detection may be inaccurate\nand pictures are not parsed");
    vert->addWidget(cBox);
    connect(cBox, &QCheckBox::stateChanged,
        [](int val) {
            MPGSETTINGS.fastscan = val == Qt::Checked;
        }
    );
}
