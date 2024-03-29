/*
 *  Copyright (C) 2020-2023 Leandro Nini
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
#include "settings.h"

#include <QDebug>

#include <limits>

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
    m_audioProcess(nullptr),
    m_audioConverter(nullptr),
    m_bytes(0),
    m_bytePerMilliSec(std::numeric_limits<int>::max()),
    m_milliSeconds(0),
    m_finished(false),
    m_maxPlayTime(song->maxPlayTime())
{}

InputWrapper::~InputWrapper()
{
    delete m_audioProcess;
    delete m_audioConverter;
}

qint64 InputWrapper::bytesAvailable() const
{
    if (m_finished)
        return 0;

    const int bytes = SETTINGS->bufLen() * m_bytePerMilliSec;
    return bytes + QIODevice::bytesAvailable();
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
        size_t const bufSize = m_audioConverter->bufSize(maxSize);
        size_t const size = m_currentSong->fillBuffer(m_audioConverter->buffer(), bufSize);
        n = m_audioConverter->convert(data, size);
    }
    else
    {
        n = m_currentSong->fillBuffer(data, maxSize);
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

    if (m_maxPlayTime && (m_milliSeconds > m_maxPlayTime))
    {
        qDebug() << "reached max playing time";
        m_finished = true;
        emit songFinished();
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
            emit songFinished();
        }
        else
        {
            qDebug() << "finished playing";
            m_finished = true;
            emit songFinished();
            return 0;
        }
    }

    m_bytes += n;

    const unsigned int oldSeconds = m_milliSeconds / 1000;

    while (m_bytes > m_bytePerMilliSec)
    {
        m_bytes -= m_bytePerMilliSec;
        m_milliSeconds++;
    }

    const unsigned int newSeconds = m_milliSeconds / 1000;

    if (oldSeconds != newSeconds)
    {
        if (newSeconds == (m_currentSong->songDuration()/1000)-5)
        {
            emit preloadSong();
        }
        emit updateTime();
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

void InputWrapper::setPosition(double pos)
{
    if (m_currentSong->seek(pos))
        m_milliSeconds = static_cast<unsigned int>(pos * m_currentSong->songDuration());
}

bool InputWrapper::setFormat(audioFormat_t format)
{
    unsigned int precision;

    switch (format.sampleType)
    {
    case sample_t::U8:
        m_audioProcess = new audioProcess8();
        precision = 1;
        break;
    case sample_t::S16:
        m_audioProcess = new audioProcess16();
        precision = 2;
        break;
    case sample_t::S32:
        m_audioProcess = new audioProcess32();
        precision = 4;
        break;
    case sample_t::SAMPLE_FLOAT:
        m_audioProcess = new audioProcessFloat();
        precision = 4;
        break;
    default:
        m_audioProcess = nullptr;
        return false;
    }

    m_bytePerMilliSec = (format.sampleRate * format.channels * precision) / 1000;

    audioFormat_t songFormat;
    songFormat.sampleRate = m_currentSong->samplerate();
    songFormat.channels = m_currentSong->channels();
    songFormat.sampleType = m_currentSong->precision();

    // Check if soundcard supports requested samplerate
    m_audioConverter = CFACTORY->get(songFormat, format, m_currentSong->fract());

    return true;
}
