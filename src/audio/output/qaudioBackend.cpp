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

const char qaudioBackend::name[] = "QAUDIO";

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

    audioBuffer.buffer().resize(0);
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
    audioBuffer.buffer().append((const char*)buffer, bufferSize);

    while ((audioBuffer.bytesAvailable() > bufferSize)
        && (_audioOutput->state() == QAudio::ActiveState))
#if QT_VERSION >= 0x050000
        QThread::msleep(1);
#else
#  ifdef _WIN32
        Sleep(1);
#  else
        usleep(1000);
#  endif
#endif

    return true;
}

void qaudioBackend::pause()
{
    _audioOutput->suspend();
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
