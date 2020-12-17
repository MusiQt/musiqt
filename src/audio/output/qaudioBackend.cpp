/*
 *  Copyright (C) 2006-2020 Leandro Nini
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
#include <QDebug>

#ifndef _WIN32
#  include <unistd.h>
#endif

/*****************************************************************/

QStringList qaudioBackend::devices()
{
    QStringList devices;
    // Check devices
    foreach(const QAudioDeviceInfo &deviceInfo, QAudioDeviceInfo::availableDevices(QAudio::AudioOutput))
    {
        devices.append(deviceInfo.deviceName().toUtf8().constData());
    }
    return devices;
}

qaudioBackend::qaudioBackend() :
    _audioOutput(nullptr)
{}

void qaudioBackend::onStateChange(QAudio::State newState)
{
    qDebug() << "onStateChange: " << newState;
    /*switch (newState)
    {
    case QAudio::IdleState:
        stop();
        break;
    case QAudio::StoppedState:
        if (_audioOutput->error() != QAudio::NoError)
        {
            qWarning() << "Error";
        }
        break;
    default:
        break;
    }*/
}

size_t qaudioBackend::open(const unsigned int card, unsigned int &sampleRate,
                           const unsigned int channels, const unsigned int prec, QIODevice* device)
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

    if (!list[card].isFormatSupported(format))
    {
        format = list[card].nearestFormat(format);
#if QT_VERSION >= 0x050000
        sampleRate = format.sampleRate();
#else
        sampleRate = format.frequency();
#endif
        //qWarning() << "Audio format not supported";
        //return 0;
    }

    _audioOutput = new QAudioOutput(list[card], format);
    if (_audioOutput->error() != QAudio::NoError)
    {
        qWarning() << "error";
        delete _audioOutput;
        _audioOutput = nullptr;
        return 0;
    }

    connect(_audioOutput, SIGNAL(stateChanged(QAudio::State)), this, SLOT(onStateChange(QAudio::State)));

    device->open(QIODevice::ReadWrite);
    _audioOutput->start(device);

    if (_audioOutput->error() != QAudio::NoError)
    {
        qWarning() << "error";
        delete _audioOutput;
        _audioOutput = nullptr;
        return 0;
    }

    qDebug() << "bufferSize: " << _audioOutput->bufferSize() << " bytes";

    return _audioOutput->bufferSize();
}

void qaudioBackend::close()
{
    delete _audioOutput;
    _audioOutput = nullptr;
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
#  if QT_VERSION >= 0x050800
    qreal volume = QAudio::convertVolume(vol / qreal(100.0),
                                         QAudio::LogarithmicVolumeScale,
                                         QAudio::LinearVolumeScale);
#  else
    qreal volume = qreal(vol/100.0f);
#  endif
    _audioOutput->setVolume(volume);
#else
    qDebug("Unimplemented");
#endif
}

int qaudioBackend::volume()
{
#if QT_VERSION >= 0x050000
#  if QT_VERSION >= 0x050800
    return QAudio::convertVolume(_audioOutput->volume(),
                                 QAudio::LinearVolumeScale,
                                 QAudio::LogarithmicVolumeScale) * 100;
#  else
    return _audioOutput->volume()*100;
#  endif
#else
    qDebug("Unimplemented");
    return 0;
#endif
}
