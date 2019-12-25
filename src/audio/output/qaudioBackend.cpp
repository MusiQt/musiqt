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

#include "qaudioBackend.h"

#include <QAudioDeviceInfo>
#include <QAudioFormat>
#include <QList>
#include <QString>
#include <QThread>
#include <QDebug>

#ifndef _WIN32
#  include <unistd.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <algorithm>

const char qaudioBackend::name[] = "QAUDIO";

/*****************************************************************/

AudioBuffer::AudioBuffer() {}
AudioBuffer::~AudioBuffer() {}

qint64 AudioBuffer::readData(char *data, qint64 maxSize)
{
    if (sem.available()==BUFFERS)
        return 0;

    const qint64 size = std::min(length[readIdx], maxSize);
    memcpy(data, buffer[readIdx].get(), size);
    const qint64 left = length[readIdx] - size;
    length[readIdx] = left;
    if (left==0)
    {
        readIdx = 1 - readIdx;
        sem.release();
    }
    else
    {
        memcpy(buffer[readIdx].get(), buffer[readIdx].get()+size, left);
    }
    return size;
}

qint64 AudioBuffer::writeData(const char *data, qint64 maxSize)
{
    if (!sem.tryAcquire(1, 100))
        return 0;

    const qint64 size = std::min(bufSize, maxSize);
    memcpy(buffer[writeIdx].get(), data, size);
    length[writeIdx] = size;
    writeIdx = 1 - writeIdx;
    return size;
}

bool AudioBuffer::isSequential() const { return true; }
qint64 AudioBuffer::bytesAvailable() const { return length[readIdx]; }

void AudioBuffer::init(qint64 size)
{
    readIdx = 0;
    writeIdx = 0;
    bufSize = size;
    length[0] = 0;
    length[1] = 0;
    buffer[0].reset(new char[size]);
    buffer[1].reset(new char[size]);
    if (sem.available()<BUFFERS)
    {
        sem.release(BUFFERS-sem.available());
    }
}

/*****************************************************************/

qaudioBackend::qaudioBackend() :
    outputBackend(name),
    _audioOutput(nullptr)
{
    _buffer = nullptr;

    // Check devices
    foreach(const QAudioDeviceInfo &deviceInfo, QAudioDeviceInfo::availableDevices(QAudio::AudioOutput))
    {
        addDevice(deviceInfo.deviceName().toUtf8().constData());
    }
}

size_t qaudioBackend::open(const unsigned int card, unsigned int &sampleRate,
                           const unsigned int channels, const unsigned int prec)
{
    QAudioFormat format;
#if QT_VERSION >= 0x050000
    format.setSampleRate(sampleRate);
    format.setChannelCount(channels);
#else
    format.setFrequency(sampleRate);
    format.setChannels(channels);
#endif
    format.setSampleSize(prec * 8);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(prec == 1 ? QAudioFormat::UnSignedInt : QAudioFormat::SignedInt);

    QList<QAudioDeviceInfo> list = QAudioDeviceInfo::availableDevices(QAudio::AudioOutput);

    _audioOutput = new QAudioOutput(list[card], format);
    if (_audioOutput->error() != QAudio::NoError)
    {
        qDebug("error");
    }

    audioBuffer.open(QIODevice::ReadWrite);
    _audioOutput->start(&audioBuffer);

    if (_audioOutput->error() != QAudio::NoError)
    {
        delete _audioOutput;
        _audioOutput = nullptr;
        return 0;
    }

    qDebug() << "bufferSize: " << _audioOutput->bufferSize() << " bytes";

    _buffer = new char[_audioOutput->bufferSize()];

    audioBuffer.init(_audioOutput->bufferSize());

    return _audioOutput->bufferSize();
}

void qaudioBackend::close()
{
    delete _audioOutput;
    _audioOutput = nullptr;

    delete [] _buffer;
}

bool qaudioBackend::write(void* buffer, size_t bufferSize)
{
loop:
    const qint64 n = audioBuffer.write((const char*)buffer, bufferSize);

    if (n < 0)
        return false;

    if (n == 0)
    {
        switch (_audioOutput->state())
        {
        case QAudio::ActiveState:
        case QAudio::IdleState:
            goto loop;
        case QAudio::SuspendedState:
        case QAudio::StoppedState:
            return true;
        }
    }

    bufferSize -= n;
    if (bufferSize > 0)
    {
        buffer = (char*)buffer+n;
        goto loop;
    }

    return true;
}

void qaudioBackend::pause()
{
    _audioOutput->suspend();
}

void qaudioBackend::unpause()
{
    _audioOutput->resume();
}

void qaudioBackend::stop()
{
    _audioOutput->stop();
}

void qaudioBackend::volume(int vol)
{
#if QT_VERSION >= 0x050000
    _audioOutput->setVolume(qreal(vol/100.0f));
#else
    qDebug("Unimplemented");
#endif
}

int qaudioBackend::volume()
{
#if QT_VERSION >= 0x050000
    return _audioOutput->volume()*100;
#else
    qDebug("Unimplemented");
    return 0;
#endif
}
