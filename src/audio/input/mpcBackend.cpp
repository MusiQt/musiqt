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

#include "mpcBackend.h"

#include "settings.h"
#include "utils.h"
#include "tag.h"

#include <QDebug>
#include <QLabel>

// created by reswrap from file musepack.gif
extern const unsigned char iconMpc[417] =
{
    0x47,0x49,0x46,0x38,0x39,0x61,0x10,0x00,0x10,0x00,0xf5,0x3c,0x00,0xc6,0x41,0x00,
    0xc6,0x45,0x00,0xc6,0x49,0x00,0xce,0x45,0x00,0xce,0x4d,0x00,0xce,0x51,0x00,0xce,
    0x59,0x00,0xce,0x61,0x08,0xce,0x65,0x10,0xce,0x61,0x18,0xd6,0x69,0x18,0xd6,0x65,
    0x21,0xd6,0x6d,0x21,0xd6,0x6d,0x29,0xd6,0x71,0x21,0xd6,0x71,0x29,0xd6,0x75,0x31,
    0xd6,0x79,0x31,0xd6,0x7d,0x39,0xd6,0x7d,0x42,0xd6,0x82,0x39,0xd6,0x86,0x4a,0xd6,
    0x8a,0x4a,0xd6,0x8e,0x52,0xde,0x8e,0x52,0xde,0x8e,0x5a,0xde,0x92,0x5a,0xd6,0x96,
    0x63,0xde,0x96,0x63,0xde,0x9a,0x63,0xde,0x9e,0x6b,0xde,0x9e,0x73,0xde,0xa2,0x73,
    0xde,0xa6,0x7b,0xde,0xaa,0x84,0xde,0xb2,0x8c,0xe7,0xb2,0x8c,0xe7,0xb2,0x94,0xe7,
    0xba,0x9c,0xe7,0xbe,0xa5,0xe7,0xc3,0xa5,0xe7,0xc7,0xad,0xe7,0xcb,0xad,0xe7,0xcb,
    0xb5,0xe7,0xd3,0xbd,0xef,0xd3,0xc6,0xef,0xd7,0xc6,0xef,0xdb,0xce,0xef,0xdf,0xd6,
    0xef,0xe3,0xd6,0xef,0xe7,0xde,0xf7,0xe3,0xd6,0xef,0xeb,0xe7,0xef,0xef,0xe7,0xef,
    0xef,0xef,0xef,0xf3,0xef,0xef,0xf7,0xf7,0xef,0xfb,0xff,0xff,0xf7,0xef,0xf7,0xff,
    0xff,0xff,0xff,0xff,0xab,0xac,0xad,0x00,0x00,0x00,0x00,0x00,0x00,0x21,0xf9,0x04,
    0x01,0x00,0x00,0x3d,0x00,0x2c,0x00,0x00,0x00,0x00,0x10,0x00,0x10,0x00,0x00,0x06,
    0xbe,0xc0,0xdd,0x8e,0x44,0xf0,0x08,0x8f,0x3b,0x0e,0xc1,0x74,0x94,0x25,0x0a,0x05,
    0x17,0xd2,0x65,0x28,0x2c,0x6a,0x42,0x14,0x41,0x41,0x08,0x21,0x45,0x84,0x04,0xe1,
    0xc4,0xdb,0x7d,0x08,0x22,0xc3,0x24,0x77,0xc4,0x14,0xc0,0x9b,0x72,0x85,0xf0,0x72,
    0x20,0x68,0x42,0x1c,0xe4,0xf0,0x32,0x3c,0xca,0x0f,0x07,0x3b,0x73,0x2f,0x42,0x36,
    0x08,0x0a,0x3b,0x0f,0x06,0x3a,0x3b,0x07,0x0c,0x3b,0x20,0x03,0x2b,0x42,0x32,0x06,
    0x10,0x3b,0x18,0x02,0x33,0x31,0x05,0x12,0x3b,0x25,0x01,0x24,0x42,0x2f,0x04,0x15,
    0x3b,0x21,0x01,0x28,0x2e,0x04,0x18,0x3b,0x2a,0x03,0x1f,0x42,0x2d,0x04,0x1a,0x3b,
    0x26,0x01,0x21,0x2b,0x03,0x1d,0x3b,0xac,0x17,0x42,0xbb,0x46,0x2c,0xb5,0x2b,0x00,
    0x5e,0x9d,0x11,0x59,0xb9,0x3b,0x9d,0x12,0x19,0x04,0x23,0x3b,0x34,0x08,0x90,0xb7,
    0x00,0xd2,0x34,0x09,0x08,0x04,0x03,0x25,0x42,0x81,0x78,0x23,0x03,0x4c,0x37,0x0d,
    0x03,0x28,0x26,0x30,0x42,0x14,0x05,0xeb,0xa9,0x29,0x3b,0x36,0x2b,0x4c,0x48,0x1a,
    0x04,0x2c,0x49,0xf8,0x48,0x47,0x37,0xa8,0xaa,0x3b,0x2c,0xd0,0x41,0x12,0x04,0x00,
    0x3b
};

#define EXT "mpc"

#define CREDITS "Musepack\nCopyright \302\251 The Musepack Development Team"
#define LINK    "http://www.musepack.com/"

const char mpcBackend::name[] = "Musepack";

/*****************************************************************/

size_t mpcBackend::fillBuffer(void* buffer, const size_t bufferSize, const unsigned int seconds)
{
    size_t n = 0;

    do {
        if (_bufIndex >= _bufLen)
        {
#ifdef SV8
            mpc_frame_info frame;
            frame.buffer = _buffer;
            const mpc_status err = mpc_demux_decode(_demux, &frame);
            if (frame.bits == -1)
                    break;
            if (err != MPC_STATUS_OK)
            {
                qWarning("Decoding error");
                return 0;
            }

            _bufLen = frame.samples*_si.channels*sizeof(MPC_SAMPLE_FORMAT);
#else
            _bufLen = mpc_decoder_decode(&_decoder, _buffer, 0, 0)*_si.channels*sizeof(MPC_SAMPLE_FORMAT);
            if (!_bufLen)
                break;
            if (_bufLen < 0)
            {
                qWarning("Decoding error");
                return 0;
            }
#endif
            _bufIndex = 0;
        }

        const size_t num = qMin((size_t)(_bufLen-_bufIndex), bufferSize-n);
        memcpy((char*)buffer+n, (char*)_buffer+_bufIndex, num);
        n += num;
        _bufIndex += num;
    } while (n < bufferSize);

    return n;
}

/*****************************************************************/

bool mpcBackend::supports(const QString& fileName)
{
    QRegExp rx("*." EXT);
    rx.setPatternSyntax(QRegExp::Wildcard);
    return rx.exactMatch(fileName);
}

inline QStringList mpcBackend::ext() const { return QStringList(EXT); }

mpcBackend::mpcBackend() :
    inputBackend(name, iconMpc, 417)
{
    _mpcReader.read = mpcBackend::read_func;
    _mpcReader.seek = mpcBackend::seek_func;
    _mpcReader.tell = mpcBackend::tell_func;
    _mpcReader.get_size = mpcBackend::get_size_func;
    _mpcReader.canseek = mpcBackend::canseek_func;
    _mpcReader.data = &_file;
}

mpcBackend::~mpcBackend()
{
    close();
}

bool mpcBackend::open(const QString& fileName)
{
    close();

    _file.setFileName(fileName);
    if (!_file.open(QIODevice::ReadOnly))
        return false;

    const tag tagPtr(&_file);

    _metaData.addInfo(metaData::TITLE, tagPtr.title());
    _metaData.addInfo(metaData::ARTIST, tagPtr.artist());
    _metaData.addInfo(metaData::ALBUM, tagPtr.album());
    _metaData.addInfo(metaData::GENRE, tagPtr.genre());
    _metaData.addInfo(metaData::YEAR, tagPtr.year());
    _metaData.addInfo(gettext("publisher"), tagPtr.publisher());
    _metaData.addInfo(metaData::TRACK, tagPtr.track());
    _metaData.addInfo(metaData::COMMENT, tagPtr.comment());
    _metaData.addInfo(tagPtr.image());

    _file.seek(0);

#ifdef SV8
    _demux = mpc_demux_init(&_mpcReader);
    if (!_demux)
        goto error;

    mpc_demux_get_info(_demux, &_si);
#else
    mpc_streaminfo_init(&_si);
    if (mpc_streaminfo_read(&_si, &_mpcReader) != ERROR_CODE_OK)
        goto error;

    mpc_decoder_setup(&_decoder, &_mpcReader);
    if (!mpc_decoder_initialize(&_decoder, &_si))
        goto error;
#endif

    time((int)mpc_streaminfo_get_length(&_si));

#ifdef MPC_FIXED_POINT
    qDebug("FIXED");
#else
    qDebug("FLOAT");
#endif

    if (SETTINGS->replayGain())
    {
        // Replaygain reference level (is this correct?)
        const double referenceLevel = 89.0;
#ifdef SV8
        mpc_set_replay_level(_demux, referenceLevel, MPC_TRUE,
                (SETTINGS->replayGainMode() == 0) ? MPC_FALSE : MPC_TRUE,
                MPC_TRUE);
#else
        float peak = SETTINGS->replayGainMode() ? _si.peak_album : _si.peak_title;
        float gain = SETTINGS->replayGainMode() ? _si.gain_album : _si.gain_title;

        peak = peak == 0. ? 1. : (1<<15) / pow(10, peak/(20*256));
        gain = gain == 0. ? 1. : pow(10, (referenceLevel-gain/256)/20);

        qDebug() << "peak: " << peak << " - gain: " << gain;

        if (peak < gain)
            gain = peak;

        mpc_decoder_scale_output(&decoder, gain);
#endif
    }

    _bufIndex = 0;
    _bufLen = 0;

    songLoaded(fileName);
    return true;

error:
    close();
    return false;
}

void mpcBackend::close()
{
    _file.close();

#ifdef SV8
    if (!songLoaded().isNull())
        mpc_demux_exit(_demux);
#endif

    songLoaded(QString::null);
}

bool mpcBackend::rewind()
{
#ifdef SV8
    if (mpc_demux_seek_sample(_demux, 0) != MPC_STATUS_OK)
#else
    if (!mpc_decoder_seek_seconds(&_decoder, 0))
#endif
        return false;

    _bufIndex = 0;
    _bufLen = 0;

    return true;
}

mpc_int32_t mpcBackend::read_func(DATAPARM, void *ptr, mpc_int32_t size)
{
    return ((QFile*)DATAFILE)->read((char*)ptr, size);
}

mpc_bool_t mpcBackend::seek_func(DATAPARM, mpc_int32_t offset)
{
    ((QFile*)DATAFILE)->seek(offset);
    return true;
}

mpc_int32_t mpcBackend::tell_func(DATAPARM)
{
    return ((QFile*)DATAFILE)->pos();
}

mpc_int32_t mpcBackend::get_size_func(DATAPARM)
{
    return ((QFile*)DATAFILE)->size();
}

mpc_bool_t mpcBackend::canseek_func(DATAPARM)
{
    return true;
}

/*****************************************************************/

mpcConfig::mpcConfig(QWidget* win) :
    configFrame(win, mpcBackend::name, CREDITS, LINK)
{
    matrix()->addWidget(new QLabel(tr("No settings available"), this));
}
