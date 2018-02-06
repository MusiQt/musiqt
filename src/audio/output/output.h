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

#ifndef OUTPUT_H
#define OUTPUT_H

#include <cstdlib>

#include <QString>

/**
 * interface class for output backends
 */
class output
{
public:
    virtual ~output() {}

    /// Get backend name
    virtual const char* name() const =0;

    /// Get number of available devices
    virtual unsigned int devices() const =0;

    /// Get device name
    virtual const QString device(const unsigned int i) const =0;

    /// Open
    /// returns the buffer size
    virtual size_t open(const unsigned int card, unsigned int &sampleRate, const unsigned int channels, const unsigned int prec) =0;

    /// Close
    virtual void close() =0;

    /// Get buffer
    virtual void* buffer() =0;

    /// Write data to output
    virtual bool write(void* buffer, size_t bufferSize) =0;

    /// Pause
    virtual void pause()=0;

    /// Unpause
    virtual void unpause()=0;

    /// Stop
    virtual void stop() =0;

    /// Set volume
    virtual void volume(int vol) =0;

    /// Get volume
    virtual int volume() const =0;
};

#endif
