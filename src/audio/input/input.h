/*
 *  Copyright (C) 2006-2023 Leandro Nini
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

#ifndef INPUT_H
#define INPUT_H

#include "inputTypes.h"
#include "metaDataImpl.h"
#include "exceptions.h"

#include <QString>

#ifdef ENABLE_NLS
#  include <libintl.h>
#endif

/**
 * base class for input backends
 */
class input
{
public:
    class loadError : public error { using error::error; };

private:
    unsigned int m_time;

protected:
    metaDataImpl m_metaData;

protected:
    input() : m_time(0) {}

    /// Song is loaded
    void songLoaded(const QString& location);

    /// Set song duration
    void setDuration(unsigned int newTime) { m_time = newTime; }

public:
    virtual ~input() = default;

    /// Get song info
    const metaData* getMetaData() const { return &m_metaData; }

    /// Get song duration in milliseconds
    unsigned int songDuration() const { return m_time; }

    /// Get max play time in milliseconds, 0 if none
    virtual unsigned int maxPlayTime() const { return 0; }

    /// Get samplerate
    virtual unsigned int samplerate() const =0;

    /// Get channels
    virtual unsigned int channels() const =0;

    /// Get precision
    virtual sample_t precision() const =0;

    /// Get fractional scale for fixed point types
    virtual unsigned int fract() const { return 0; }

    /// Rewind to start
    virtual bool rewind() { return seek(0); }

    /// Seek support
    virtual bool seekable() const { return false; }

    /// Seek specified position
    virtual bool seek([[maybe_unused]] double pos) { return false; }

    /// Callback function
    virtual size_t fillBuffer(void* buffer, size_t bufferSize) =0;

    /// Song is loaded
    QString songLoaded() const { return m_metaData.getInfo(metaData::URL); }

    /// Get number of subtunes
    virtual unsigned int subtunes() const { return 0; }

    /// Get current subtune
    virtual unsigned int subtune() const { return 0; }

    /// Change subtune
    virtual bool subtune([[maybe_unused]] unsigned int i) { return false; }

    /// Gapless support
    virtual bool gapless() const { return false; }
};

#endif
