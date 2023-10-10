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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "wvBackend.h"

#include "settings.h"
#include "utils.h"

#include <QDebug>

/* created by reswrap from file wavpack.gif */
extern const unsigned char iconWv[375] =
{
    0x47,0x49,0x46,0x38,0x39,0x61,0x10,0x00,0x10,0x00,0xf5,0x00,0x00,0x00,0x00,0x00,
    0x00,0x01,0x00,0x03,0x04,0x06,0x04,0x06,0x08,0x05,0x06,0x08,0x07,0x09,0x0d,0x07,
    0x0a,0x0e,0x07,0x0a,0x0f,0x07,0x0b,0x0f,0x07,0x0b,0x10,0x08,0x0b,0x10,0x0a,0x0c,
    0x14,0x0b,0x10,0x17,0x0e,0x14,0x1d,0x0d,0x15,0x1f,0x0e,0x16,0x1f,0x10,0x17,0x22,
    0x13,0x1c,0x28,0x14,0x1e,0x2b,0x15,0x1e,0x2b,0x17,0x21,0x30,0x19,0x25,0x37,0x1a,
    0x24,0x36,0x1e,0x2c,0x41,0x20,0x2f,0x45,0x21,0x2f,0x45,0x20,0x30,0x45,0x26,0x38,
    0x51,0x2a,0x3c,0x57,0x2e,0x42,0x61,0x30,0x45,0x65,0x34,0x4d,0x70,0x36,0x4f,0x72,
    0x38,0x51,0x77,0x39,0x52,0x77,0x3e,0x5a,0x83,0x3e,0x5c,0x87,0x41,0x5f,0x8b,0x44,
    0x64,0x91,0x45,0x67,0x95,0x47,0x69,0x98,0x48,0x6a,0x99,0x4a,0x6c,0x9e,0x4b,0x6d,
    0x9f,0x4b,0x6e,0xa2,0x4d,0x72,0xa5,0x4f,0x73,0xa6,0x4f,0x74,0xaa,0x50,0x76,0xab,
    0x52,0x77,0xad,0x52,0x77,0xae,0x52,0x79,0xae,0x56,0x7f,0xb9,0x5e,0x87,0xc4,0x61,
    0x8e,0xce,0x61,0x8f,0xd0,0x62,0x8f,0xd0,0x61,0x90,0xd0,0x62,0x90,0xd1,0x63,0x90,
    0xd2,0x62,0x91,0xd2,0x63,0x91,0xd3,0x64,0x93,0xd4,0x00,0x00,0x00,0x21,0xf9,0x04,
    0x01,0x00,0x00,0x3f,0x00,0x2c,0x00,0x00,0x00,0x00,0x10,0x00,0x10,0x00,0x00,0x06,
    0x94,0xc0,0x9f,0xc1,0xa0,0x50,0x24,0x0c,0x89,0x64,0x51,0x31,0x14,0xda,0x6c,0xa1,
    0xd6,0x13,0xf5,0xc4,0x7d,0x64,0x4f,0xe2,0x53,0x10,0xb1,0xe1,0x02,0x2a,0x5b,0x29,
    0xf0,0xb4,0x25,0x14,0x4f,0x0d,0xf9,0x13,0x18,0xd9,0x02,0x96,0xa7,0x4f,0x0b,0x25,
    0x07,0x02,0x9c,0x19,0xd9,0xd6,0xf3,0x9d,0x9f,0x2a,0x01,0x1d,0x0b,0x1e,0x0e,0x13,
    0x18,0x36,0x3a,0x59,0x68,0x4f,0x77,0x30,0x29,0x77,0x2c,0x65,0x66,0x8c,0x6f,0x0d,
    0x36,0x35,0x77,0x77,0x24,0x59,0x09,0x65,0x01,0x58,0x34,0x7b,0x04,0x0b,0x4f,0x7f,
    0x8a,0x26,0x4f,0x17,0x04,0x8d,0x88,0x36,0x5a,0x8a,0x38,0x8d,0x1b,0x32,0x14,0x7b,
    0x94,0x93,0x36,0x28,0x9a,0x71,0x65,0x74,0x5e,0x36,0x86,0xc0,0xc3,0x7f,0x65,0x5f,
    0x27,0x5e,0x3d,0x3a,0x3d,0x59,0x06,0x72,0x36,0x2d,0x10,0xb9,0xbe,0x42,0x43,0x48,
    0x4c,0xd6,0xd9,0x3f,0x41,0x00,0x3b
};

#define EXT "wv"

#define	BUFFER_LENGTH 4096

#define CREDITS "WAVPACK<br>Hybrid Lossless Wavefile Compressor<br>Copyright \302\251 Conifer Software."
#define LINK    "http://www.wavpack.com/"

const char wvBackend::name[] = "Wavpack";

inputConfig* wvBackend::cFactory() { return new wvConfig(name, iconWv, 375); }

/*****************************************************************/

size_t wvBackend::fillBuffer(void* buffer, const size_t bufferSize)
{
    size_t n = 0;
    const unsigned int sampleSize = (m_precision == sample_t::U8) ? 1 : (m_precision == sample_t::S16) ? 2 : 4;

    do {
        if (m_bufOffset >= m_bufSize)
        {
            m_bufSize = WavpackUnpackSamples(m_wvContext, m_decodeBuf, BUFFER_LENGTH) * m_channels;
            if (m_bufSize == 0)
            {
                return n;
            }
            m_bufOffset = 0;
        }
        const size_t size = qMin((size_t)(m_bufSize-m_bufOffset), (bufferSize-n) / sampleSize);
        copyBuffer((char*)buffer+n, m_decodeBuf+m_bufOffset, size);
        m_bufOffset += size;
        n += size*sampleSize;
    } while (n < bufferSize);

    return bufferSize;
}

void wvBackend::copyBuffer(char* dest, const int* src, size_t length)
{
    switch (m_precision)
    {
    case sample_t::U8:
        for (size_t i=0; i<length; i++)
            dest[i] = 128 + (char)(src[i]);
        break;
    case sample_t::S16:
        for (size_t i=0; i<length; i++)
        {
            dest[i<<1] = (char)(src[i]);
            dest[(i<<1)+1] = (char)(src[i]>>8);
        }
        break;
    default:
        memcpy(dest, src, length*4);
    }
}

/*****************************************************************/

QStringList wvBackend::ext() { return QStringList(EXT); }

wvBackend::wvBackend(const QString& fileName) :
    m_config(name, iconWv, 375)
{
    char tmp[255];
    m_wvContext = WavpackOpenFileInput(fileName.toUtf8().constData(), tmp, OPEN_WVC|OPEN_TAGS|OPEN_2CH_MAX|OPEN_NORMALIZE, 0);
    if (m_wvContext == nullptr)
    {
        throw loadError(QString("Error: %1").arg(tmp));
    }

    const int mode = WavpackGetMode(m_wvContext);
    m_bps = WavpackGetBytesPerSample(m_wvContext);
    m_channels = WavpackGetReducedChannels(m_wvContext);

    m_decodeBuf = new int[BUFFER_LENGTH*m_channels];

    if (mode & MODE_FLOAT)
    {
        m_precision = sample_t::SAMPLE_FLOAT;
    }
    else
    {
        switch (m_bps)
        {
        case 1:
            m_precision = sample_t::U8;
            break;
        case 2:
            m_precision = sample_t::S16;
            break;
        default:
            m_precision = sample_t::SAMPLE_FIXED;
        }
    }

    if (mode & MODE_VALID_TAG)
    {
        if (mode & MODE_APETAG)
        {
            // APEv2
            getApeTag("Title", metaData::TITLE);
            getApeTag("Artist", metaData::ARTIST);
            getApeTag("Album", metaData::ALBUM);
            getApeTag("Year", metaData::CONTENT_CREATED);
            getApeTag("Track", metaData::TRACK_NUMBER);
            getApeTag("Comment", metaData::COMMENT);
            getApeTag("Genre", metaData::GENRE);

            int size = WavpackGetBinaryTagItem(m_wvContext, "Cover Art", nullptr, 0);
            if (size)
            {
                char *buffer = new char[size];
                WavpackGetBinaryTagItem(m_wvContext, "Cover Art", buffer, size);
                m_metaData.addInfo(new QByteArray(buffer, size));
                delete [] buffer;
            }
        }
        else
        {
            // ID3v1
            getId3Tag("title", metaData::TITLE);
            getId3Tag("artist", metaData::ARTIST);
            getId3Tag("album", metaData::ALBUM);
            getId3Tag("year", metaData::CONTENT_CREATED);
            getId3Tag("track", metaData::TRACK_NUMBER);
            getId3Tag("comment", metaData::COMMENT);
        }
    }

    const unsigned int milliseconds = (WavpackGetNumSamples(m_wvContext) * 1000LL) / WavpackGetSampleRate(m_wvContext);
    setDuration(milliseconds);

    m_bufOffset = 0;
    m_bufSize = 0;

    songLoaded(fileName);
}

wvBackend::~wvBackend()
{
    m_wvContext = WavpackCloseFile(m_wvContext);
    delete [] m_decodeBuf;
}

void wvBackend::getId3Tag(const char* tag, metaData::mpris_t meta)
{
    char tmp[255];
    int size = WavpackGetTagItem(m_wvContext, tag, tmp, 255);
    if (size >= 0)
        m_metaData.addInfo(meta, QString::fromLocal8Bit(tmp, size));
}

void wvBackend::getApeTag(const char* tag, metaData::mpris_t meta)
{
    char tmp[1024];
    int size = WavpackGetTagItem(m_wvContext, tag, tmp, 1024);
    if (size >= 0)
        m_metaData.addInfo(meta, QString::fromUtf8(tmp, size).replace('\0', ','));
}

bool wvBackend::seek(double pos)
{

#if WAVPACK_VERSION >= 5
    int64_t samples = WavpackGetNumSamples64(m_wvContext);
    if (samples == -1)
    {
        qWarning() << "Error getting number of samples:" << WavpackGetErrorMessage(m_wvContext);
        return false;
    }

    int64_t sample = samples * pos;
    if (!WavpackSeekSample64(m_wvContext, sample))
    {
        qWarning() << "Error seeking:" << WavpackGetErrorMessage(m_wvContext);
        return false;
    }
#else
    uint32_t samples = WavpackGetNumSamples(m_wvContext);
    if (samples == (uint32_t)-1)
    {
        qWarning() << "Error getting number of samples:" << WavpackGetErrorMessage(m_wvContext);
        return false;
    }

    uint32_t sample = samples * pos;
    if (!WavpackSeekSample(m_wvContext, sample))
    {
        qWarning() << "Error seeking:" << WavpackGetErrorMessage(m_wvContext);
        return false;
    }
#endif
    m_bufOffset = 0;
    m_bufSize = 0;

    return true;
}

/*****************************************************************/

wvConfigFrame::wvConfigFrame(QWidget* win) :
    configFrame(win, CREDITS, LINK)
{
    matrix()->addWidget(new QLabel(tr("No settings available"), this));
}
