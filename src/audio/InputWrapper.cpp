/*
 *  Copyright (C) 2020-2021 Leandro Nini
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

#if defined(PROFILE)
#  include <QElapsedTimer>

#  define PROFILE_START QElapsedTimer timer; \
    timer.start();
#  define PROFILE_END   qDebug() << "Elapsed " << timer.elapsed();
#else
#  define PROFILE_START
#  define PROFILE_END
#endif

InputWrapper::InputWrapper(input* song) :
    m_currentSong(song),
    m_preloadedSong(nullptr),
    m_audioConverter(nullptr),
    m_bytes(0),
    m_milliSeconds(0)
{}

InputWrapper::~InputWrapper()
{
    delete m_audioProcess;

    if (m_audioConverter != nullptr)
        delete m_audioConverter;
}

void InputWrapper::enableBs2b()
{
    if (m_audioProcess != nullptr)
        m_audioProcess->init(m_currentSong->samplerate());
}

qint64 InputWrapper::fillBuffer(char *data, qint64 maxSize)
{
PROFILE_START
    size_t n;

    if (m_audioConverter != nullptr)
    {
        size_t size = m_currentSong->fillBuffer(
            m_audioConverter->buffer(),
            m_audioConverter->bufSize(maxSize),
            m_milliSeconds);
        n = m_audioConverter->convert(data, size);
    }
    else
    {
        n = m_currentSong->fillBuffer(data, maxSize, m_milliSeconds);
    }

    m_audioProcess->process(data, n);
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
        if (m_preloadedSong != nullptr)
        {
            m_currentSong = m_preloadedSong;
            m_preloadedSong = nullptr;
            m_milliSeconds = 0;
            n = fillBuffer(data, maxSize);
            emit switchSong();
        }
        else
        {
            qDebug() << "finished playing";
            return 0;
        }
    }

    m_bytes += n;

    const int oldSeconds = m_milliSeconds / 1000;

    do {
        m_bytes -= m_bytePerMilliSec;
        m_milliSeconds++;
    } while (m_bytes > m_bytePerMilliSec);

    const int newSeconds = m_milliSeconds / 1000;

    if (oldSeconds != newSeconds)
    {
        if (newSeconds != (m_currentSong->songDuration()/1000)-5)
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
    if ((m_currentSong->samplerate() == newSong->samplerate())
        && (m_currentSong->channels() == newSong->channels())
        && (m_currentSong->precision() == newSong->precision()))
    {
        m_preloadedSong = newSong;
        return true;
    }
    else
    {
        m_preloadedSong = nullptr;
        return false;
    }
}

void InputWrapper::unload()
{
    m_preloadedSong = nullptr;
}

void InputWrapper::seek(int pos)
{
    if (m_currentSong->seek(pos))
        m_milliSeconds = (pos * m_currentSong->songDuration()) / 100;
}

int InputWrapper::tell() const
{
    return (100*m_milliSeconds)/m_currentSong->songDuration();
}

void InputWrapper::setFormat(int sampleRate, int channels, sample_t sampleType, int bufferSize)
{
    unsigned int precision;

    switch (sampleType)
    {
    case sample_t::U8:
        m_audioProcess = new audioProcess8();
        precision = 1;
        break;
    case sample_t::S16:
        m_audioProcess = new audioProcess16();
        precision = 2;
        break;
    default:
        m_audioProcess = nullptr;
        // This should not happen
        qFatal("Unsupported sample type");
        break;
    }

    m_bytePerMilliSec = (sampleRate * channels * precision) / 1000;

    // QIODevice has a fixed buffer of 16Kb
    bufferSize = 16384;

    // Check if soundcard supports requested samplerate
    m_audioConverter = CFACTORY->get(m_currentSong->samplerate(), sampleRate, bufferSize,
        m_currentSong->channels(), m_currentSong->precision(), sampleType, m_currentSong->fract());
}
