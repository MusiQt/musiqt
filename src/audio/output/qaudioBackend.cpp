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

#include "qaudioBackend.h"

#include <QAudioDeviceInfo>
#include <QAudioFormat>
#include <QList>
#include <QDebug>
#include <QThreadPool>

/*****************************************************************/

void deviceLoader::run()
{
    qaudioBackend::getDevices();
}

/*****************************************************************/

QStringList devices;

QStringList qaudioBackend::getDevices()
{
    if (devices.empty())
    {
        // Check devices
        for (const QAudioDeviceInfo &deviceInfo: QAudioDeviceInfo::availableDevices(QAudio::AudioOutput))
        {
            devices.append(deviceInfo.deviceName().toUtf8().constData());
            qDebug() << "Device name: " << deviceInfo.deviceName();
            qDebug() << "SampleRates: " << deviceInfo.supportedSampleRates();
            qDebug() << "SampleSizes: " << deviceInfo.supportedSampleSizes();
        }
    }

    return devices;
}

qaudioBackend::qaudioBackend() :
    audioOutput(nullptr)
{
    // Preload available devices in a separate thread
    deviceLoader* loader = new deviceLoader();
    loader->setAutoDelete(true);
    QThreadPool::globalInstance()->start(loader);
}

void qaudioBackend::onStateChange(QAudio::State newState)
{
    qDebug() << "onStateChange: " << newState;
    switch (newState)
    {
    case QAudio::IdleState:
        emit songEnded();
        break;
    case QAudio::StoppedState:
        if (audioOutput->error() != QAudio::NoError)
        {
            qWarning() << "Error " << audioOutput->error();
        }
        break;
    default:
        break;
    }
}

size_t qaudioBackend::open(const unsigned int card, unsigned int &sampleRate,
                           const unsigned int channels, const sample_t sType, QIODevice* device)
{
    int sampleSize;
    QAudioFormat::SampleType sampleType;

    switch (sType)
    {
    case sample_t::U8:
        sampleSize = 8;
        sampleType = QAudioFormat::UnSignedInt;
        break;
    case sample_t::S16:
        sampleSize = 16;
        sampleType = QAudioFormat::SignedInt;
        break;
    case sample_t::S24:
        sampleSize = 24;
        sampleType = QAudioFormat::SignedInt;
        break;
    case sample_t::S32:
        sampleSize = 32;
        sampleType = QAudioFormat::SignedInt;
        break;
    default:
        qWarning() << "Unsupported sample type";
        return 0;
    }

    QAudioFormat format;
#if QT_VERSION >= 0x050000
    format.setSampleRate(sampleRate);
    format.setChannelCount(channels);
#else
    format.setFrequency(sampleRate);
    format.setChannels(channels);
#endif
    format.setSampleSize(sampleSize);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(sampleType);

    QList<QAudioDeviceInfo> list = QAudioDeviceInfo::availableDevices(QAudio::AudioOutput);

    if (!list[card].isFormatSupported(format))
    {
        format = list[card].nearestFormat(format);
#if QT_VERSION >= 0x050000
        sampleRate = format.sampleRate();
#else
        sampleRate = format.frequency();
#endif
        // FIXME
        qWarning() << "Audio format not supported";
        return 0;
    }

    audioOutput = new QAudioOutput(list[card], format);
    if (audioOutput->error() != QAudio::NoError)
    {
        qWarning() << "Error creating QAudioOutput";
        delete audioOutput;
        audioOutput = nullptr;
        return 0;
    }

    connect(audioOutput, SIGNAL(stateChanged(QAudio::State)), this, SLOT(onStateChange(QAudio::State)));

    device->open(QIODevice::ReadOnly);
    audioOutput->start(device);

    if (audioOutput->error() != QAudio::NoError)
    {
        qWarning() << "Error starting QAudioOutput";
        delete audioOutput;
        audioOutput = nullptr;
        return 0;
    }

    // suspend audio playback until initialization is done
    audioOutput->suspend();

    return audioOutput->bufferSize();
}

void qaudioBackend::close()
{
    delete audioOutput;
    audioOutput = nullptr;
}

void qaudioBackend::pause()
{
    audioOutput->suspend();
}

void qaudioBackend::unpause()
{
    audioOutput->resume();
}

void qaudioBackend::stop()
{
    audioOutput->stop();
}

void qaudioBackend::volume(int vol)
{
    if (audioOutput == nullptr)
        return;

#if QT_VERSION >= 0x050000
#  if QT_VERSION >= 0x050800
    qreal volume = QAudio::convertVolume(vol / qreal(100.0),
                                         QAudio::LogarithmicVolumeScale,
                                         QAudio::LinearVolumeScale);
#  else
    qreal volume = qreal(vol/100.0f);
#  endif
    audioOutput->setVolume(volume);
#else
    qDebug("Unimplemented");
#endif
}

int qaudioBackend::volume()
{
    if (audioOutput == nullptr)
        return 0;

#if QT_VERSION >= 0x050000
#  if QT_VERSION >= 0x050800
    return QAudio::convertVolume(audioOutput->volume(),
                                 QAudio::LinearVolumeScale,
                                 QAudio::LogarithmicVolumeScale) * 100;
#  else
    return audioOutput->volume()*100;
#  endif
#else
    qDebug("Unimplemented");
    return 0;
#endif
}
