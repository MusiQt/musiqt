/*
 *  Copyright (C) 2006-2017
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

#ifndef OSS_H
#define OSS_H

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#if defined (HAVE_SYS_SOUNDCARD_H) || defined (HAVE_MACHINE_SOUNDCARD_H)

#include "outputBackend.h"

/*****************************************************************/

/**
 * OSS output backend
 */
class ossBackend final : public outputBackend
{
private:
    int _audioHandle;

    void *_buffer;

private:
    ossBackend();

    void testDevice(const char* devName);

    size_t setParams(unsigned int &sampleRate, const unsigned int channels, const unsigned int prec);

public:
    virtual ~ossBackend() {}

    static const char name[];

    /// Factory method
    static output* factory() { return new ossBackend(); }

    /// Open
    size_t open(const unsigned int card, unsigned int &sampleRate,
                const unsigned int channels, const unsigned int prec) override;

    /// Close
    void close() override;

    /// Get buffer
    void *buffer() override { return _buffer; }

    /// Write data to output
    bool write(void* buffer, size_t bufferSize) override;

    /// Pause
    void pause() override;

    /// Stop
    void stop() override;

    /// Set volume
    void volume(int vol) override;

    /// Get volume
    int volume() override;
};

#endif

#endif
