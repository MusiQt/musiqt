/*
 *  Copyright (C) 2009-2017 Leandro Nini
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
#include "loadsym.h"

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

#define CREDITS "MPEG Audio Decoder library\ncopyright 1995-2008 by the mpg123 project"
#define LINK    "http://mpg123.org/"

const char mpg123Backend::name[]="Mpg123";

#ifdef _WIN32
# define MPG123LIB	"libmpg123-0.dll"
#else
# define MPG123LIB	"libmpg123.so"
#endif

int mpg123Backend::_status=0;
const AutoDLL mpg123Backend::_dll(MPG123LIB);

int (*mpg123Backend::dl_mpg123_init)(void)=0;
void (*mpg123Backend::dl_mpg123_exit)(void)=0;
mpg123_handle* (*mpg123Backend::dl_mpg123_new)(const char*, int*)=0;
void (*mpg123Backend::dl_mpg123_delete)(mpg123_handle*)=0;
const char* (*mpg123Backend::dl_mpg123_plain_strerror)(int)=0;
int (*mpg123Backend::dl_mpg123_open_fd)(mpg123_handle*, int)=0;
int (*mpg123Backend::dl_mpg123_close)(mpg123_handle*)=0;
int (*mpg123Backend::dl_mpg123_scan)(mpg123_handle*)=0;
int (*mpg123Backend::dl_mpg123_getformat)(mpg123_handle*, long*, int*, int*)=0;
off_t (*mpg123Backend::dl_mpg123_length)(mpg123_handle*)=0;
int (*mpg123Backend::dl_mpg123_read)(mpg123_handle*, unsigned char*, size_t, size_t*)=0;
off_t (*mpg123Backend::dl_mpg123_seek)(mpg123_handle*, off_t, int)=0;
int (*mpg123Backend::dl_mpg123_id3)(mpg123_handle*, mpg123_id3v1**, mpg123_id3v2**)=0;
const char** (*mpg123Backend::dl_mpg123_supported_decoders)(void)=0;
int(*mpg123Backend::dl_mpg123_replace_reader)(mpg123_handle*, ssize_t(*)(int, void *, size_t), off_t(*)(int, off_t, int))=0;
int (*mpg123Backend::dl_mpg123_param)(mpg123_handle*, enum mpg123_parms, long, double)=0;

QStringList mpg123Backend::_decoders;

QFile mpg123Backend::_file[2];
int mpg123Backend::_count=0;

mpg123Config_t mpg123Backend::_settings;

/*****************************************************************/

size_t mpg123Backend::fillBuffer(void* buffer, const size_t bufferSize, const unsigned int seconds)
{
    size_t n;
    const int err=dl_mpg123_read(_handle, (unsigned char*)buffer, bufferSize, &n);
    if (err!=MPG123_OK)
        return 0;

    return n;
}

/*****************************************************************/

bool mpg123Backend::init()
{
    if (!_dll.loaded())
        return false;

    LOADSYM(_dll, mpg123_init, int(*)(void))
    LOADSYM(_dll, mpg123_exit, void(*)(void))
    LOADSYM(_dll, mpg123_new, mpg123_handle*(*)(const char*, int*))
    LOADSYM(_dll, mpg123_delete, void(*)(mpg123_handle*))
    LOADSYM(_dll, mpg123_plain_strerror, const char*(*)(int))
    LOADSYMSFX(_dll, mpg123_open_fd, int(*)(mpg123_handle*, int))
    LOADSYM(_dll, mpg123_close, int(*)(mpg123_handle*))
    LOADSYM(_dll, mpg123_scan, int(*)(mpg123_handle*))
    LOADSYM(_dll, mpg123_getformat, int(*)(mpg123_handle*, long*, int*, int*))
    LOADSYMSFX(_dll, mpg123_length, off_t(*)(mpg123_handle*))
    LOADSYM(_dll, mpg123_read, int(*)(mpg123_handle*, unsigned char*, size_t, size_t*))
    LOADSYMSFX(_dll, mpg123_seek, off_t(*)(mpg123_handle*, off_t, int))
    LOADSYM(_dll, mpg123_id3, int(*)(mpg123_handle*, mpg123_id3v1**, mpg123_id3v2**))
    LOADSYM(_dll, mpg123_supported_decoders, const char**(*)(void))
    TRYLOADSYM(_dll, mpg123_replace_reader, mpg123_replace_reader_64, int(*)(mpg123_handle*, ssize_t(*)(int, void *, size_t), off_t(*)(int, off_t, int)))
    LOADSYM(_dll, mpg123_param, int(*)(mpg123_handle*, enum mpg123_parms, long, double))

    return true;
}

bool mpg123Backend::supports(const QString& fileName)
{
    QRegExp rx("*." EXT);
    rx.setPatternSyntax(QRegExp::Wildcard);
    return rx.exactMatch(fileName);
}

inline QStringList mpg123Backend::ext() const { return QStringList(EXT); }

mpg123Backend::mpg123Backend() :
    inputBackend(name, iconMpg123, 560),
    _handle(nullptr)
{
    loadSettings();

    if (++_status>1)
        return;

    const int err=dl_mpg123_init();
    if (err==MPG123_OK)
    {
        _decoders << "auto";
        const char** decoders=dl_mpg123_supported_decoders();
        while (*decoders)
        {
            qDebug() << "Decoder: " << *decoders;
            _decoders << *decoders;
            ++decoders;
        }
    }
    else
    {
        qWarning() << dl_mpg123_plain_strerror(err);
        _status=0;
    }
}

mpg123Backend::~mpg123Backend()
{
    close();

    if (--_status==0)
        dl_mpg123_exit();
}

void mpg123Backend::loadSettings()
{
    _settings.fastscan=load("Fastscan", true);
    _settings.decoder=load("Decoder", "auto");
}

void mpg123Backend::saveSettings()
{
    save("Fastscan", _settings.fastscan);
    save("Decoder", _settings.decoder);
}

bool mpg123Backend::open(const QString& fileName)
{
    if (!_status)
        return false;

    close();

    _fd = _count&1;
    _file[_fd].setFileName(fileName);
    if (!_file[_fd].open(QIODevice::ReadOnly))
        return false;

    int err;
    qDebug() << "Setting decoder: " << _settings.decoder;
    const char *decoder = QString::compare(_settings.decoder, "auto")
        ? _settings.decoder.toStdString().c_str()
        : nullptr;
    _handle = dl_mpg123_new(decoder, &err);
    if (_handle == nullptr)
        goto error;

    err = dl_mpg123_replace_reader(_handle, read_func, seek_func);
    if (err == MPG123_OK)
    {
        err = dl_mpg123_open_fd(_handle, _fd);
    }
    if (err != MPG123_OK)
    {
        dl_mpg123_delete(_handle);
        goto error;
    }

    if (!_settings.fastscan)
    {
        err = dl_mpg123_scan(_handle);
        if (err!=MPG123_OK)
        {
            qWarning() << dl_mpg123_plain_strerror(err);
        }
    }

    int encoding;
    err = dl_mpg123_getformat(_handle, &_samplerate, &_channels, &encoding);
    if (err!=MPG123_OK)
    {
        dl_mpg123_delete(_handle);
        goto error;
    }

    err = dl_mpg123_length(_handle);
    time(err/_samplerate);

    err=dl_mpg123_param(_handle, MPG123_RVA,
        SETTINGS->replayGain()
            ? (SETTINGS->replayGainMode()==0) ? MPG123_RVA_ALBUM : MPG123_RVA_MIX
            : MPG123_RVA_OFF,
        0.);

    mpg123_id3v1* id3v1;
    mpg123_id3v2* id3v2;
    err = dl_mpg123_id3(_handle, &id3v1, &id3v2);
    if (err == MPG123_OK)
    {
        QString info;

        if (id3v2 && id3v2->title)
            info = QString::fromUtf8(id3v2->title->p);
        else if (id3v1)
        {
            info = QString(id3v1->title);
            info.resize(std::min<int>(strlen(id3v1->title), 30));
            info = info.trimmed();
        }
        else
            info=QString::null;
        qDebug() << "TITLE: " << info;
        _metaData.addInfo(metaData::TITLE, info);

        if (id3v2 && id3v2->artist)
            info = QString::fromUtf8(id3v2->artist->p);
        else if (id3v1)
        {
            info = QString(id3v1->artist);
            info.resize(std::min<int>(strlen(id3v1->artist), 30));
            info = info.trimmed();
        }
        else
            info = QString::null;
        qDebug() << "ARTIST: " << info;
        _metaData.addInfo(metaData::ARTIST, info);

        if (id3v2 && id3v2->album)
            info = QString::fromUtf8(id3v2->album->p);
        else if (id3v1)
        {
            info = QString(id3v1->album);
            info.resize(std::min<int>(strlen(id3v1->album), 30));
            info = info.trimmed();
        }
        else
            info=QString::null;
        qDebug() << "ALBUM: " << info;
        _metaData.addInfo(metaData::ALBUM, info);

        if (id3v2 && id3v2->genre)
        {
            info = QString::fromUtf8(id3v2->genre->p);
            int st = info.indexOf('(');
            if (st>=0)
            {
                QStringRef tmp = info.midRef(st,info.indexOf(')')-st);
                const unsigned int idx=tmp.string()->toInt();
                if (idx<GENRES)
                    info=QString(genre[idx]);
            }
        }
        else if (id3v1 && id3v1->genre<GENRES)
            info = genre[id3v1->genre];
        else
            info=QString::null;
        qDebug() << "GENRE: " << info;
        _metaData.addInfo(metaData::GENRE, info);

        if (id3v2 && id3v2->year)
            info = QString::fromUtf8(id3v2->year->p);
        else if (id3v1)
        {
            info = QString(id3v1->year);
            info.resize(std::min<int>(strlen(id3v1->year), 4));
            info=info.trimmed();
        }
        else
            info = QString::null;
        qDebug() << "YEAR: " << info;
        _metaData.addInfo(metaData::YEAR, info);

        if (id3v1 && id3v1->comment[28]==0)
            info = QString::number(id3v1->comment[29]);
        else
            info = QString::null;
        qDebug() << "TRACK: " << info;
        _metaData.addInfo(metaData::TRACK, info);

        info=QString::null;
        if (id3v2)
        {
            for (unsigned int i=0; i<id3v2->comments; i++)
            {
                mpg123_text *entry = &id3v2->comment_list[i];
                if (entry->description.fill==0 || entry->description.p[0]==0)
                    info = QString::fromUtf8(entry->text.p).trimmed();
            }
        }

        if (info.isEmpty() && id3v1)
        {
            info = QString(id3v1->comment);
            info.resize(std::min<int>(strlen(id3v1->comment), 30));
            info = info.trimmed();
        }

        if (!info.isEmpty())
        {
            qDebug() << "COMMENT: " << info;
            _metaData.addInfo(metaData::COMMENT, info);
        }
    }

    _count++;

    songLoaded(fileName);
    return true;

error:
    qWarning() << dl_mpg123_plain_strerror(err);
    _file[_fd].close();
    return false;
}

void mpg123Backend::close()
{
    if (songLoaded().isEmpty())
        return;

    dl_mpg123_close(_handle);
    dl_mpg123_delete(_handle);

    _file[_fd].close();

    songLoaded(QString::null);
}

bool mpg123Backend::rewind()
{
    if (!songLoaded().isEmpty())
    {
        dl_mpg123_seek(_handle, 0, SEEK_SET);
        return true;
    }

    return false;
}

ssize_t mpg123Backend::read_func(int fd, void* ptr, size_t size)
{
    return _file[fd].read((char*)ptr, size);
}

off_t mpg123Backend::seek_func(int fd, off_t offset, int whence)
{
    qint64 pos;
    switch (whence)
    {
    case SEEK_SET:
            pos = offset;
            break;
    case SEEK_CUR:
            pos = _file[fd].pos() + offset;
            break;
    case SEEK_END:
            pos = _file[fd].size() + offset;
            break;
    default:
            return -1;
    }
    return _file[fd].seek(pos) ? pos : -1;
}

/*****************************************************************/

#define MPGSETTINGS	mpg123Backend::_settings

mpg123Config::mpg123Config(QWidget* win) :
    configFrame(win, mpg123Backend::name, CREDITS, LINK)
{
    matrix()->addWidget(new QLabel(tr("Decoder"), this));
    QComboBox *decBox = new QComboBox(this);
    matrix()->addWidget(decBox);
    decBox->addItems(mpg123Backend::_decoders);
    const int numItems = decBox->count();
    decBox->setMaxVisibleItems(std::min(numItems, 5));
    const int curItem = decBox->findText(MPGSETTINGS.decoder);
    if (curItem >= 0)
        decBox->setCurrentIndex(curItem);
    connect(decBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onCmdDecoder(int)));

    {
        QFrame* line = new QFrame();
        line->setFrameShape(QFrame::VLine);
        line->setFrameShadow(QFrame::Sunken);
        extraLeft()->addWidget(line);
    }

    QVBoxLayout *vert = new QVBoxLayout();
    extraLeft()->addLayout(vert);
    QCheckBox *cBox = new QCheckBox(tr("Fast scan"));
    cBox->setToolTip("Scan only few frame of file, time detection may be inaccurate");
    cBox->setCheckState(MPGSETTINGS.fastscan ? Qt::Checked : Qt::Unchecked);
    vert->addWidget(cBox);
    connect(cBox, SIGNAL(stateChanged(int)), this, SLOT(onCmdFastscan(int)));
}

void mpg123Config::onCmdDecoder(int val)
{
    MPGSETTINGS.decoder = mpg123Backend::_decoders.at(val);
}

void mpg123Config::onCmdFastscan(int val)
{
    MPGSETTINGS.fastscan = val == Qt::Checked;
}
