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
#include <QBuffer>
#include <QByteArray>
#include <QMutex>

#include "outputBackend.h"

/*****************************************************************/

/**
 * QAudio output backend
 */
class qaudioBackend : public outputBackend
{
private:
    QAudioOutput *_audioOutput;

    QBuffer *_audioBuffer;
    QByteArray data;

    QMutex mutex[2];

    char *_buffer[2];
    unsigned int _idx;

private:
    qaudioBackend();

public:
    virtual ~qaudioBackend() {};

    static const char name[];

    /// Factory method
    static output* factory() { return new qaudioBackend(); }

    /// Open
    size_t open(const unsigned int card, unsigned int &sampleRate, const unsigned int channels, const unsigned int prec);

    /// Close
    void close();

    /// Get buffer
    void *buffer() { return _buffer[_idx]; }

    /// Write data to output
    bool write(void* buffer, size_t bufferSize);

    /// Pause
    void pause() override;

    /// Stop
    void stop();

    /// Set volume
    void volume(int vol);

    /// Get volume
    int volume();
};

#endif
