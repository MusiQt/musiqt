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
    inputBackend(name),
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
    QComboBox *decBox=new QComboBox(this);
    matrix()->addWidget(decBox);
    decBox->addItems(mpg123Backend::_decoders);
    const int numItems=decBox->count();
    decBox->setMaxVisibleItems(std::min(numItems, 5));
    const int curItem=decBox->findText(MPGSETTINGS.decoder);
    if (curItem>=0)
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
