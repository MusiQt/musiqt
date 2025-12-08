/*
 *  Copyright (C) 2006-2025 Leandro Nini
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
#include "exceptions.h"

#include <QAudio>
#include <QPointer>
#include <QRunnable>
#include <QThread>

/*****************************************************************/

class deviceLoader : public QRunnable
{
public:
    void run() override;
};

/*****************************************************************/

struct device_t
{
    QString name;
    QString id;

    inline bool operator==(const device_t &other) {
        return this->id.compare(other.id) == 0;
    }
};

inline bool operator==(const device_t &lhs, const QString &rhs) {
    return lhs.id.compare(rhs) == 0;
}

using deviceList_t = QList<device_t>;

/**
 * QAudio output backend
 */
class qaudioBackend : public QObject
{
    Q_OBJECT

public:
    class initError : public error { using error::error; };

private:
    QPointer<AudioOutputWrapper> m_audioOutput;

    // audio thread
    QThread *m_thread;

signals:
    void songEnded();
    void audioError(const QString&);

private:
    void onStateChange(QAudio::State newState);

public:
    qaudioBackend();

    ~qaudioBackend() override;

    static deviceList_t getDevices();

    /// init audio
    /// @throws initError
    audioFormat_t init(int card, audioFormat_t format);

    /// Start audio
    void start(QIODevice* device);

    /// Close
    void close();

    /// Pause
    void pause();
    void unpause();

    /// Stop
    void stop();

    /// Set volume
    void setVolume(int vol);

    /// Get volume
    int getVolume();
};

#endif
