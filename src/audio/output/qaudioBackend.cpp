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

    // audio thread
    thread = new QThread();
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
    format.setSampleRate(sampleRate);
    format.setChannelCount(channels);
    format.setSampleSize(sampleSize);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(sampleType);

    QList<QAudioDeviceInfo> list = QAudioDeviceInfo::availableDevices(QAudio::AudioOutput);

    if (!list[card].isFormatSupported(format))
    {
        format = list[card].nearestFormat(format);
        sampleRate = format.sampleRate();

        // FIXME
        qWarning() << "Audio format not supported";
        return 0;
    }

    audioOutput = new AudioOutputWrapper();
    
    audioOutput->moveToThread(thread);
    thread->start();

    QMetaObject::invokeMethod(audioOutput, "init", Q_ARG(QAudioDeviceInfo, list[card]), Q_ARG(QAudioFormat, format));

    QAudio::Error error;
    QMetaObject::invokeMethod(audioOutput, "error", Qt::BlockingQueuedConnection, Q_RETURN_ARG(QAudio::Error, error));
    if (error != QAudio::NoError)
    {
        qWarning() << "Error creating QAudioOutput";
        delete audioOutput;
        audioOutput = nullptr;
        return 0;
    }

    connect(audioOutput, &AudioOutputWrapper::stateChanged, this, &qaudioBackend::onStateChange);

    device->open(QIODevice::ReadOnly);
    QMetaObject::invokeMethod(audioOutput, "start", Q_ARG(QIODevice*, device));

    if (audioOutput->error() != QAudio::NoError)
    {
        qWarning() << "Error starting QAudioOutput";
        delete audioOutput;
        audioOutput = nullptr;
        return 0;
    }

    // suspend audio playback until initialization is done
    QMetaObject::invokeMethod(audioOutput, "suspend");

    int bufSize;
    QMetaObject::invokeMethod(audioOutput, "bufferSize", Qt::BlockingQueuedConnection, Q_RETURN_ARG(int, bufSize));
    return bufSize;
}

void qaudioBackend::close()
{
    thread->quit();
    thread->wait();
    
    delete audioOutput;
    audioOutput = nullptr;
}

void qaudioBackend::pause()
{
    QMetaObject::invokeMethod(audioOutput, "suspend");
}

void qaudioBackend::unpause()
{
    QMetaObject::invokeMethod(audioOutput, "resume");
}

void qaudioBackend::stop()
{
    QMetaObject::invokeMethod(audioOutput, "stop");
}

void qaudioBackend::volume(int vol)
{
    if (audioOutput == nullptr)
        return;

#if QT_VERSION >= 0x050800
    qreal volume = QAudio::convertVolume(vol / qreal(100.0),
                                         QAudio::LogarithmicVolumeScale,
                                         QAudio::LinearVolumeScale);
#else
    qreal volume = qreal(vol/100.0f);
#endif
    QMetaObject::invokeMethod(audioOutput, "setVolume", Q_ARG(qreal, volume));
}

int qaudioBackend::volume()
{
    if (audioOutput == nullptr)
        return 0;

#if QT_VERSION >= 0x050800
    return QAudio::convertVolume(audioOutput->volume(),
                                 QAudio::LinearVolumeScale,
                                 QAudio::LogarithmicVolumeScale) * 100;
#else
    qreal volume;
    QMetaObject::invokeMethod(audioOutput, "volume", Qt::BlockingQueuedConnection, Q_RETURN_ARG(qreal, volume));
    return volume*100;
#endif
}
