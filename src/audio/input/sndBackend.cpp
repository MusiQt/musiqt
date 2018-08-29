/*
 *  Copyright (C) 2006-2018 Leandro Nini
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

#include "sndBackend.h"

#include "settings.h"

#include <QDebug>
#include <QLabel>

#define CREDITS "libsndfile\nCopyright \302\251 Erik de Castro Lopo"
#define LINK    "http://www.mega-nerd.com/libsndfile/"

QStringList sndBackend::_ext;

const char sndBackend::name[] = "Sndfile";

/*****************************************************************/

size_t sndBackend::fillBuffer(void* buffer, const size_t bufferSize, const unsigned int seconds)
{
    return (sf_read_short(_sf, (short*)buffer, (bufferSize>>1))<<1);
}

/*****************************************************************/

bool sndBackend::init()
{
    unsigned int count;
    sf_command(0, SFC_GET_SIMPLE_FORMAT_COUNT, &count, sizeof(int));

    SF_FORMAT_INFO format_info;
    for (unsigned int i=0; i<count; i++)
    {
        format_info.format = i;
        sf_command(0, SFC_GET_SIMPLE_FORMAT, &format_info, sizeof(format_info));
        if (!_ext.contains(format_info.extension))
        {
            _ext << format_info.extension;
        }
    }

    qDebug() << "Sndfile extensions supported: " << _ext;
    return true;
}

bool sndBackend::supports(const QString& fileName)
{
    QString ext = _ext.join("|");
    ext.prepend(".*\\.(").append(")");
    qDebug() << "sndBackend::supports: " << ext;

    QRegExp rx(ext);
    return rx.exactMatch(fileName);
}

sndBackend::~sndBackend()
{
    close();
}

bool sndBackend::open(const QString& fileName)
{
    close();

    _si.format = 0;
#if defined (_WIN32) && defined (UNICODE)
    const wchar_t *buffer = utils::convertUtf(fileName);

#ifdef ENABLE_SNDFILE_WINDOWS_PROTOTYPES
    _sf = sf_wchar_open(buffer, SFM_READ, &_si);
#else
    _sf = sf_open((char*)buffer, SFM_READ, &_si);
#endif

    delete [] buffer;
#else
    _sf = sf_open(fileName.toUtf8().constData(), SFM_READ, &_si);
#endif
    if (_sf == nullptr)
    {
        qWarning() <<  sf_strerror(0);
        return false;
    }

    sf_command(_sf, SFC_SET_SCALE_FLOAT_INT_READ, 0, SF_TRUE);

    _metaData.addInfo(metaData::TITLE, sf_get_string(_sf, SF_STR_TITLE));
    _metaData.addInfo(metaData::ARTIST, sf_get_string(_sf, SF_STR_ARTIST));
    _metaData.addInfo(metaData::ALBUM, sf_get_string(_sf, SF_STR_ALBUM));
    _metaData.addInfo(metaData::YEAR, sf_get_string(_sf, SF_STR_DATE));
    _metaData.addInfo(gettext("copyright"), sf_get_string(_sf, SF_STR_COPYRIGHT));
    _metaData.addInfo(metaData::COMMENT, sf_get_string(_sf, SF_STR_COMMENT));
    _metaData.addInfo(metaData::TRACK, sf_get_string(_sf, SF_STR_TRACKNUMBER));
    _metaData.addInfo(metaData::GENRE, sf_get_string(_sf, SF_STR_GENRE));

    const int seconds = _si.frames / _si.samplerate;
    time((seconds > 0x7FFFFFFF) ? 0 : seconds);

    songLoaded(fileName);
    return true;
}

void sndBackend::close()
{
    if (_sf != nullptr)
    {
        sf_close(_sf);
        _sf = nullptr;
    }

    songLoaded(QString::null);
}

bool sndBackend::rewind()
{
    if (_sf == nullptr)
        return false;

    if (sf_seek(_sf, 0, SEEK_SET) < 0)
    {
        qWarning() << sf_strerror(_sf);
        return false;
    }

    return true;
}

/*****************************************************************/

sndConfig::sndConfig(QWidget* win) :
    configFrame(win, sndBackend::name, CREDITS, LINK)
{
    matrix()->addWidget(new QLabel(tr("No settings available")));
}
