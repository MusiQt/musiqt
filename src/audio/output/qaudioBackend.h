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

#ifndef QAUDIOBACKEND_H
#define QAUDIOBACKEND_H

#include <QAudioOutput>
#include <QSemaphore>

#include <memory>

/*****************************************************************/

class AudioBuffer : public QIODevice
{
public:
    AudioBuffer();
    ~AudioBuffer();

    qint64 readData(char *data, qint64 maxSize) override;
    qint64 writeData(const char *data, qint64 maxSize) override;

    bool isSequential() const override;
    qint64 bytesAvailable() const override;

    void init(qint64 size);

private:
    static constexpr int BUFFERS=2;

    std::unique_ptr<char[]> buffer[BUFFERS];
    qint64 length[BUFFERS];
    QSemaphore sem;
    int readIdx;
    int writeIdx;
    qint64 bufSize;
};

/*****************************************************************/

/**
 * QAudio output backend
 */
class qaudioBackend
{
private:
    QAudioOutput *_audioOutput;

    AudioBuffer audioBuffer;

    char *_buffer;

public:
    qaudioBackend();

    virtual ~qaudioBackend() {};

    static QStringList devices();

    /// Open
    size_t open(const unsigned int card, unsigned int &sampleRate, const unsigned int channels, const unsigned int prec);

    /// Close
    void close();

    /// Get buffer
    void *buffer() { return _buffer; }

    /// Write data to output
    bool write(void* buffer, size_t bufferSize);

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
