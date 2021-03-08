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

#ifndef QAUDIOBACKEND_H
#define QAUDIOBACKEND_H

#include "inputTypes.h"
#include "AudioOutputWrapper.h"

#include <QAudio>
#include <QRunnable>
#include <QThread>

/*****************************************************************/

class deviceLoader : public QObject, public QRunnable
{
public:
    void run() override;
};

/*****************************************************************/

/**
 * QAudio output backend
 */
class qaudioBackend : public QObject
{
    Q_OBJECT

private:
    AudioOutputWrapper *m_audioOutput;
    QThread *m_thread;

signals:
    void songEnded();

private:
    void onStateChange(QAudio::State newState);

public:
    qaudioBackend();

    virtual ~qaudioBackend() { delete m_thread; }

    static QStringList getDevices();

    /// Open
    size_t open(const unsigned int card, unsigned int &sampleRate, const unsigned int channels, const sample_t sType, QIODevice* device);

    /// Close
    void close();

    /// Pause
    void pause();
    void unpause();

    /// Stop
    void stop();

    /// Set volume
    void volume(int vol);

    /// Get volume
    int volume();
};

#endif
