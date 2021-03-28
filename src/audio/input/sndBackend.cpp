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

#include "sndBackend.h"

#include "settings.h"

#include <QDebug>
#include <QLabel>

#define CREDITS "libsndfile<br>Copyright \302\251 Erik de Castro Lopo"
#define LINK    "http://www.mega-nerd.com/libsndfile/"

QStringList sndBackend::m_ext;

const char sndBackend::name[] = "Sndfile";

inputConfig* sndBackend::cFactory() { return new sndConfig(name); }

/*****************************************************************/

size_t sndBackend::fillBuffer(void* buffer, const size_t bufferSize)
{
    return (sf_read_short(m_sf, (short*)buffer, (bufferSize>>1))<<1);
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
        if (!m_ext.contains(format_info.extension))
        {
            m_ext << format_info.extension;
        }
    }

    return true;
}

sndBackend::sndBackend(const QString& fileName) :
    m_config(name)
{
    m_si.format = 0;
#if defined (_WIN32) && defined (UNICODE)
    const wchar_t *buffer = utils::convertUtf(fileName);

#ifdef ENABLE_SNDFILE_WINDOWS_PROTOTYPES
    m_sf = sf_wchar_open(buffer, SFM_READ, &m_si);
#else
    m_sf = sf_open((char*)buffer, SFM_READ, &m_si);
#endif

    delete [] buffer;
#else
    m_sf = sf_open(fileName.toUtf8().constData(), SFM_READ, &m_si);
#endif
    if (m_sf == nullptr)
    {
        qWarning() <<  sf_strerror(0);
        throw loadError();
    }

    sf_command(m_sf, SFC_SET_SCALE_FLOAT_INT_READ, 0, SF_TRUE);

    m_metaData.addInfo(metaData::TITLE, sf_get_string(m_sf, SF_STR_TITLE));
    m_metaData.addInfo(metaData::ARTIST, sf_get_string(m_sf, SF_STR_ARTIST));
    m_metaData.addInfo(metaData::ALBUM, sf_get_string(m_sf, SF_STR_ALBUM));
    m_metaData.addInfo(metaData::YEAR, sf_get_string(m_sf, SF_STR_DATE));
    m_metaData.addInfo(gettext("copyright"), sf_get_string(m_sf, SF_STR_COPYRIGHT));
    m_metaData.addInfo(metaData::COMMENT, sf_get_string(m_sf, SF_STR_COMMENT));
    m_metaData.addInfo(metaData::TRACK, sf_get_string(m_sf, SF_STR_TRACKNUMBER));
    m_metaData.addInfo(metaData::GENRE, sf_get_string(m_sf, SF_STR_GENRE));

    const unsigned int milliseconds = (m_si.frames * 1000LL) / m_si.samplerate;
    setDuration(milliseconds);

    songLoaded(fileName);
}

sndBackend::~sndBackend()
{
    if (m_sf != nullptr)
    {
        sf_close(m_sf);
    }
}

bool sndBackend::seek(int pos)
{
    if (m_sf == nullptr)
        return false;

    sf_count_t frames = (m_si.frames * pos) / 100;
    if (sf_seek(m_sf, frames, SEEK_SET) < 0)
    {
        qWarning() << sf_strerror(m_sf);
        return false;
    }

    return true;
}

/*****************************************************************/

sndConfigFrame::sndConfigFrame(QWidget* win) :
    configFrame(win, sndBackend::name, CREDITS, LINK)
{
    matrix()->addWidget(new QLabel(tr("No settings available")));
}
