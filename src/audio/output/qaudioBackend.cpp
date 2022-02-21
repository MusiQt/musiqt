/*
 *  Copyright (C) 2006-2022 Leandro Nini
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

#include <QtGlobal>

#if QT_VERSION >= 0x060000
#  include <QAudioDevice>
#  include <QMediaDevices>
#else
#  include <QAudioDeviceInfo>
#  include <QAudioFormat>
#endif
#include <QList>
#include <QDebug>
#include <QThreadPool>

/*****************************************************************/

void deviceLoader::run()
{
    qaudioBackend::getDevices();
}

/*****************************************************************/

#if QT_VERSION >= 0x060000

QList<QAudioDevice> devices;

QStringList qaudioBackend::getDevices()
{
    if (devices.empty())
    {
        devices = QMediaDevices::audioOutputs();
        for (const QAudioDevice &device: devices)
        {
            qDebug() << "Device id: " << device.id();
            qDebug() << "Device name: " << device.description();
            qDebug() << "SampleRate: " << device.minimumSampleRate() << " - " << device.maximumSampleRate();
            qDebug() << "ChannelCount: " << device.minimumChannelCount() << " - "  << device.maximumChannelCount();
        }
    }

    QStringList deviceNames;

    for (const QAudioDevice &device: devices)
    {
        deviceNames.append(device.description());
    }

    return deviceNames;
}

#else

QList<QAudioDeviceInfo> devices;

QStringList qaudioBackend::getDevices()
{
    if (devices.empty())
    {
        devices = QAudioDeviceInfo::availableDevices(QAudio::AudioOutput);
        for (const QAudioDeviceInfo &deviceInfo: devices)
        {
            qDebug() << "Device name: " << deviceInfo.deviceName();
            qDebug() << "SampleRates: " << deviceInfo.supportedSampleRates();
            qDebug() << "SampleSizes: " << deviceInfo.supportedSampleSizes();
        }
    }

    QStringList deviceNames;

    for (const QAudioDeviceInfo &deviceInfo: devices)
    {
        deviceNames.append(deviceInfo.deviceName().toUtf8().constData());
    }

    return deviceNames;
}

#endif

qaudioBackend::qaudioBackend() :
    m_thread(new QThread())
{
    // Preload available devices in a separate thread
    deviceLoader* loader = new deviceLoader();
    loader->setAutoDelete(true);
    QThreadPool::globalInstance()->start(loader);
}

qaudioBackend::~qaudioBackend() { delete m_thread; }

void qaudioBackend::onStateChange(QAudio::State newState)
{
    qDebug() << "onStateChange: " << newState;
    switch (newState)
    {
    case QAudio::IdleState:
        emit songEnded();
        break;
    case QAudio::StoppedState:
        if (m_audioOutput->error() != QAudio::NoError)
        {
            qWarning() << "Error " << m_audioOutput->error();
        }
        break;
    default:
        break;
    }
}

size_t qaudioBackend::open(int card, audioFormat_t format, QIODevice* device, audioFormat_t& outputFormat)
{
#if QT_VERSION >= 0x060000
    QAudioFormat::SampleFormat sampleFormat;

    switch (format.sampleType)
    {
    case sample_t::U8:
        sampleFormat = QAudioFormat::UInt8;
        break;
    case sample_t::S16:
        sampleFormat = QAudioFormat::Int16;
        break;
    case sample_t::S32:
        sampleFormat = QAudioFormat::Int32;
        break;
    case sample_t::SAMPLE_FLOAT:
        sampleFormat = QAudioFormat::Float;
        break;
    default:
        qWarning() << "Unexpected sample type";
        return 0;
    }
#else
    int sampleSize;
    QAudioFormat::SampleType sampleType;

    switch (format.sampleType)
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
    case sample_t::SAMPLE_FLOAT:
        sampleSize = 32;
        sampleType = QAudioFormat::Float;
        break;
    default:
        qWarning() << "Unexpected sample type";
        return 0;
    }
#endif
    QAudioFormat qFormat;
    qFormat.setSampleRate(format.sampleRate);
    qFormat.setChannelCount(format.channels);
#if QT_VERSION >= 0x060000
    qFormat.setSampleFormat(sampleFormat);
#else
    qFormat.setSampleSize(sampleSize);
    qFormat.setSampleType(sampleType);
    qFormat.setCodec("audio/pcm");
    qFormat.setByteOrder(QAudioFormat::LittleEndian);
#endif

#if QT_VERSION >= 0x060000
    QAudioDevice deviceInfo = card != -1 ? devices[card] : QMediaDevices::defaultAudioOutput();
    if (!deviceInfo.isFormatSupported(qFormat))
    {
        qWarning() << "Audio format not supported";
        return 0;
    }
    else
        outputFormat = format;
#else
    QAudioDeviceInfo deviceInfo = card != -1 ? devices[card] : QAudioDeviceInfo::defaultInputDevice();

    if (!deviceInfo.isFormatSupported(qFormat))
    {
        qFormat = deviceInfo.nearestFormat(qFormat);
        outputFormat.sampleRate = qFormat.sampleRate();
        outputFormat.channels = qFormat.channelCount();
        if ((qFormat.sampleType() == QAudioFormat::UnSignedInt)
            && (qFormat.sampleSize() == 8))
        {
            outputFormat.sampleType = sample_t::U8;
        }
        else if ((qFormat.sampleType() == QAudioFormat::SignedInt)
            && (qFormat.sampleSize() == 16))
        {
            outputFormat.sampleType = sample_t::S16;
        }
        else if ((qFormat.sampleType() == QAudioFormat::SignedInt)
            && (qFormat.sampleSize() == 24))
        {
            outputFormat.sampleType = sample_t::S24;
        }
        else if ((qFormat.sampleType() == QAudioFormat::SignedInt)
            && (qFormat.sampleSize() == 32))
        {
            outputFormat.sampleType = sample_t::S32;
        }
        else if ((qFormat.sampleType() == QAudioFormat::Float)
            && (qFormat.sampleSize() == 32))
        {
            outputFormat.sampleType = sample_t::SAMPLE_FLOAT;
        }
        else
        {
            qWarning() << "Audio format not supported";
            return 0;
        }
    }
    else
        outputFormat = format;
#endif
    m_audioOutput = new AudioOutputWrapper();
    
    m_audioOutput->moveToThread(m_thread);
    m_thread->start();
#if QT_VERSION >= 0x060000
    QMetaObject::invokeMethod(m_audioOutput, "init", Q_ARG(QAudioDevice, deviceInfo), Q_ARG(QAudioFormat, qFormat));
#else
    QMetaObject::invokeMethod(m_audioOutput, "init", Q_ARG(QAudioDeviceInfo, deviceInfo), Q_ARG(QAudioFormat, qFormat));
#endif

    QAudio::Error error;
    QMetaObject::invokeMethod(m_audioOutput, "error", Qt::BlockingQueuedConnection, Q_RETURN_ARG(QAudio::Error, error));
    if (error != QAudio::NoError)
    {
        qWarning() << "Error creating QAudioOutput";
        m_audioOutput->deleteLater();
        return 0;
    }

    connect(m_audioOutput, &AudioOutputWrapper::stateChanged, this, &qaudioBackend::onStateChange);

    device->open(QIODevice::ReadOnly);
    QMetaObject::invokeMethod(m_audioOutput, "start", Q_ARG(QIODevice*, device));

    if (m_audioOutput->error() != QAudio::NoError)
    {
        qWarning() << "Error starting QAudioOutput";
        m_audioOutput->deleteLater();
        return 0;
    }

    // suspend audio playback until initialization is done
    QMetaObject::invokeMethod(m_audioOutput, "suspend");

    int bufSize;
    QMetaObject::invokeMethod(m_audioOutput, "bufferSize", Qt::BlockingQueuedConnection, Q_RETURN_ARG(int, bufSize));
    if (bufSize <= 0) {
        qWarning() << "Error getting buffer size: " << bufSize;
        m_audioOutput->deleteLater();
        return 0;
    }
    return bufSize;
}

void qaudioBackend::close()
{
    m_thread->quit();
    m_thread->wait();

    m_audioOutput->deleteLater();
}

void qaudioBackend::pause()
{
    QMetaObject::invokeMethod(m_audioOutput, "suspend");
}

void qaudioBackend::unpause()
{
    QMetaObject::invokeMethod(m_audioOutput, "resume");
}

void qaudioBackend::stop()
{
    QMetaObject::invokeMethod(m_audioOutput, "stop");
}

void qaudioBackend::setVolume(int vol)
{
    if (m_audioOutput.isNull())
        return;

    qreal volume = QAudio::convertVolume(vol / qreal(100.0),
                                         QAudio::LogarithmicVolumeScale,
                                         QAudio::LinearVolumeScale);

    QMetaObject::invokeMethod(m_audioOutput, "setVolume", Q_ARG(qreal, volume));
}

int qaudioBackend::getVolume()
{
    if (m_audioOutput.isNull())
        return 0;

    return QAudio::convertVolume(m_audioOutput->volume(),
                                 QAudio::LinearVolumeScale,
                                 QAudio::LogarithmicVolumeScale) * 100;
}
