/*
 *  Copyright (C) 2021 Leandro Nini
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

#ifndef AUDIOOUTPUTWRAPPER_H
#define AUDIOOUTPUTWRAPPER_H

#include <QtGlobal>

#if QT_VERSION >= 0x060000

#include <QMediaDevices>
#include <QAudioSink>

    Q_DECLARE_METATYPE(QAudioDevice)

class AudioOutputWrapper : public QObject
{
    Q_OBJECT

private:
    QAudioSink *m_audioOutput;

public slots:
    void init(QAudioDevice audioDevice, QAudioFormat format)
    {
        m_audioOutput = new QAudioSink(audioDevice, format, this);
        connect(m_audioOutput, SIGNAL(stateChanged(QAudio::State)), this, SLOT(onStateChange(QAudio::State)));
    }
    QAudio::Error error() const { return m_audioOutput->error(); }
    void suspend() { m_audioOutput->suspend(); }
    void resume() { m_audioOutput->resume(); }
    void start(QIODevice *device) { m_audioOutput->start(device); }
    void stop() { m_audioOutput->stop(); }
    qsizetype bufferSize() const { return m_audioOutput->bufferSize(); }
    void setVolume(qreal volume) { m_audioOutput->setVolume(volume); }
    qreal volume() const { return m_audioOutput->volume(); }

    void onStateChange(QAudio::State state) { emit stateChanged(state); }

signals:
    void stateChanged(QAudio::State state);

public:
    AudioOutputWrapper() :
        m_audioOutput(nullptr)
    {}
    ~AudioOutputWrapper() { delete m_audioOutput; }
};

#else

#include <QAudioOutput>

    Q_DECLARE_METATYPE(QIODevice*)

class AudioOutputWrapper : public QObject
{
    Q_OBJECT

private:
    QAudioOutput *m_audioOutput;

public slots:
    void init(QAudioDeviceInfo audioDevice, QAudioFormat format)
    {
        m_audioOutput = new QAudioOutput(audioDevice, format, this);
        connect(m_audioOutput, SIGNAL(stateChanged(QAudio::State)), this, SLOT(onStateChange(QAudio::State)));
    }
    QAudio::Error error() const { return m_audioOutput->error(); }
    void suspend() { m_audioOutput->suspend(); }
    void resume() { m_audioOutput->resume(); }
    void start(QIODevice *device) { m_audioOutput->start(device); }
    void stop() { m_audioOutput->stop(); }
    int bufferSize() const { return m_audioOutput->bufferSize(); }
    void setVolume(qreal volume) { m_audioOutput->setVolume(volume); }
    qreal volume() const { return m_audioOutput->volume(); }

    void onStateChange(QAudio::State state) { emit stateChanged(state); }

signals:
    void stateChanged(QAudio::State state);

public:
    AudioOutputWrapper() :
        m_audioOutput(nullptr)
    {}
    ~AudioOutputWrapper() override { delete m_audioOutput; }
};

#endif

#endif
