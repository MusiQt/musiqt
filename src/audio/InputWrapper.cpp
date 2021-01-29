/*
 *  Copyright (C) 2020 Leandro Nini
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

#include "InputWrapper.h"

#include "input/input.h"
#include "output/audioProcess.h"
#include "converter/converterFactory.h"

#include <QDebug>

//#define PROFILE

#if defined(PROFILE) && QT_VERSION >= 0x040700
#  include <QElapsedTimer>

#  define PROFILE_START QElapsedTimer timer; \
    timer.start();
#  define PROFILE_END   qDebug() << "Elapsed " << timer.elapsed();
#else
#  define PROFILE_START
#  define PROFILE_END
#endif

InputWrapper::InputWrapper(input* song) :
    currentSong(song),
    preloadedSong(nullptr),
    audioConverter(nullptr),
    bytes(0),
    seconds(0)
{}

InputWrapper::~InputWrapper()
{
    delete aProcess;

    if (audioConverter != nullptr)
        delete audioConverter;
}

void InputWrapper::enableBs2b()
{
    if (aProcess != nullptr)
        aProcess->init(currentSong->samplerate());
}

qint64 InputWrapper::fillBuffer(char *data, qint64 maxSize)
{
PROFILE_START
    size_t n;

    if (audioConverter != nullptr)
    {
        size_t size = currentSong->fillBuffer(
            audioConverter->buffer(),
            audioConverter->bufSize(maxSize),
            seconds);
        n = audioConverter->convert(data, size);
    }
    else
    {
        n = currentSong->fillBuffer(data, maxSize, seconds);
    }

    aProcess->process(data, n);
PROFILE_END

    return n;
}

qint64 InputWrapper::readData(char *data, qint64 maxSize)
{
    if (maxSize == 0)
    {
        qDebug() << "readData maxSize=0";
        return 0;
    }

    size_t n = fillBuffer(data, maxSize);

    if (n == 0)
    {
        if (preloadedSong != nullptr)
        {
            currentSong = preloadedSong;
            preloadedSong = nullptr;
            seconds = 0;
            n = fillBuffer(data, maxSize);
            emit switchSong();
        }
        else
        {
            qDebug() << "finished playing";
            return 0;
        }
    }

    bytes += n;
    if (bytes > bytePerSec)
    {
        do {
            bytes -= bytePerSec;
            seconds++;
        } while (bytes > bytePerSec);
        if (seconds != currentSong->time()-5)
            emit updateTime();
        else
            emit preloadSong();
    }

    return n;
}

qint64 InputWrapper::writeData(const char *data, qint64 maxSize)
{
    Q_UNUSED(data);
    Q_UNUSED(maxSize);

    return 0;
}

bool InputWrapper::tryPreload(input* newSong)
{
    if ((currentSong->samplerate() == newSong->samplerate())
        && (currentSong->channels() == newSong->channels())
        && (currentSong->precision() == newSong->precision()))
    {
        preloadedSong = newSong;
        return true;
    }
    else
    {
        preloadedSong = nullptr;
        return false;
    }
}

void InputWrapper::unload()
{
    preloadedSong = nullptr;
}

void InputWrapper::setFormat(int sampleRate, int channels, sample_t sampleType, int bufferSize)
{
    unsigned int precision;

    switch (sampleType)
    {
    case sample_t::U8:
        aProcess = new audioProcess8();
        precision = 1;
        break;
    case sample_t::S16:
        aProcess = new audioProcess16();
        precision = 2;
        break;
    default:
        aProcess = nullptr;
        // This should not happen
        qFatal("Unsupported sample type");
        break;
    }

    bytePerSec = sampleRate * channels * precision;

    // Check if soundcard supports requested samplerate
    audioConverter = CFACTORY->get(currentSong->samplerate(), sampleRate, bufferSize,
        currentSong->channels(), currentSong->precision(), sampleType, currentSong->fract());
}
