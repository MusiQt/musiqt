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

#include "ossBackend.h"

#if defined (HAVE_SYS_SOUNDCARD_H) || defined (HAVE_MACHINE_SOUNDCARD_H)

#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <errno.h>

#ifdef HAVE_SYS_SOUNDCARD_H
#  include <sys/soundcard.h>
#elif defined HAVE_MACHINE_SOUNDCARD_H
#  include <machine/soundcard.h>
#endif

#include <QDebug>

#define MAX_DEVICES 8

const char ossBackend::name[] = "OSS";

/*****************************************************************/

ossBackend::ossBackend() :
    outputBackend(name),
    _audioHandle(0),
    _buffer(nullptr)
{
    // Check devices
    const char devBase[] = "/dev/dsp";

    testDevice(devBase);
    for (int num=0; num<MAX_DEVICES; num++)
    {
        const QString tmp = QString("%1%2").arg(devBase).arg(num);
        testDevice(tmp.toLocal8Bit().constData());
    }
}

void ossBackend::testDevice(const char* devName)
{
    if (!access(devName, W_OK))
        addDevice(devName);
}

size_t ossBackend::open(const unsigned int card, unsigned int &sampleRate,
                        const unsigned int channels, const unsigned int prec)
{
    // Open card
    const QString devName = device(card);
    _audioHandle = ::open(devName.toLocal8Bit().constData(), O_WRONLY, 0);
    if (_audioHandle == -1)
    {
        qWarning() << "Error: " << strerror(errno);
        return 0;
    }

    // Set parameters
    const int bufSize = setParams(sampleRate, channels, prec);
    if (!bufSize)
    {
        ::close(_audioHandle);
        _audioHandle = 0;
        return 0;
    }

    int mask = 0;
    if (ioctl(_audioHandle, SOUND_MIXER_READ_DEVMASK, &mask) == -1)
    {
        qDebug() << "No mixer available";
    }

    if (mask & (1<<SOUND_MIXER_PCM))
    {
        qDebug() << "PCM is supported";
    }

    return bufSize;
}

void ossBackend::close()
{
    ::close(_audioHandle);
    _audioHandle = 0;

    free(_buffer);
}

size_t ossBackend::setParams(unsigned int &sampleRate, const unsigned int channels, const unsigned int prec)
{
    // Format
    int format = 0;
    switch (prec)
    {
    case 1:
        format = AFMT_U8;
        break;
    case 2:
        format = AFMT_S16_LE;
        break;
#ifdef AFMT_S32_LE
    case 3:
    case 4:
        format = AFMT_S32_LE;
        break;
#endif
    default:
        qWarning() << "Unsupported format";
        return 0;
    }

    if (ioctl(_audioHandle, SNDCTL_DSP_SETFMT, &format) == -1)
    {
        qWarning() << "Error: " << strerror(errno);
        return 0;
    }
    //TODO check returned format

    // Channels
    int chn = channels;
    if (ioctl(_audioHandle, SNDCTL_DSP_CHANNELS, &chn) == -1)
    {
        qWarning() << "Error: " << strerror(errno);
        return 0;
    }

    if (chn != channels)
    {
        qWarning() << "Unsupported number of channels";
        return 0;
    }

    // Samplerate
    if (ioctl(_audioHandle, SNDCTL_DSP_SPEED, &sampleRate) == -1)
    {
        qWarning() << "Error: " << strerror(errno);
        return 0;
    }

    // Block size
    int bufferSize;
    if (ioctl(_audioHandle, SNDCTL_DSP_GETBLKSIZE, &bufferSize) == -1)
    {
        qWarning() << "Error: " << strerror(errno);
        return false;
    }
    qDebug() << "Buffer size=" << bufferSize << " bytes";

    _buffer = malloc(bufferSize);

    return _buffer ? bufferSize : 0;
}

void ossBackend::pause()
{
    ioctl(_audioHandle, SNDCTL_DSP_POST, 0);
}

bool ossBackend::write(void* buffer, size_t bufferSize)
{
loop:
    const int n = ::write(_audioHandle, buffer, bufferSize);
    if (n < 0)
    {
        if (errno == EINTR)
            goto loop;
        else
            return false;
    }

    bufferSize -= n;
    if (bufferSize == 0)
        return true;

    buffer = (char*)buffer+n;
    goto loop;
}

void ossBackend::stop()
{
    ioctl(_audioHandle, SNDCTL_DSP_RESET, 0);
}

void ossBackend::volume(int vol)
{
    vol |= (vol<<8);
    if (ioctl(_audioHandle, MIXER_WRITE(SOUND_MIXER_PCM), &vol) == -1)
    {
        qDebug() << "Error: " << strerror(errno);
    }
}

int ossBackend::volume() const
{
    int vol = -1;
    if (ioctl(_audioHandle, MIXER_READ(SOUND_MIXER_PCM), &vol) == -1)
    {
        qDebug() << "Error: " << strerror(errno);
    }
    else
    {
        vol &= 0xFF;
    }
    qDebug() << "vol: " << vol;
    return vol;
}

#endif
