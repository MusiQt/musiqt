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

#ifndef MPC_H
#define MPC_H

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#ifdef MPCDEC_SV8
#  include <mpc/mpcdec.h>
#  define DATAPARM mpc_reader *p_reader
#  define DATAFILE (p_reader->data)
#else
#  include <mpcdec/mpcdec.h>
#  define DATAPARM void *data
#  define DATAFILE data
#endif

#include "input.h"

#include <QFile>

/*****************************************************************/

#include "configFrame.h"

class mpcConfigFrame : public configFrame
{
private:
    mpcConfigFrame() {}
    mpcConfigFrame(const mpcConfigFrame&);
    mpcConfigFrame& operator=(const mpcConfigFrame&);

public:
    mpcConfigFrame(QWidget* win);
    virtual ~mpcConfigFrame() {}
};

/*****************************************************************/

class mpcConfig : public inputConfig
{
public:
    mpcConfig(const char name[], const unsigned char* iconType, unsigned int iconLen) :
        inputConfig(name, iconType, iconLen)
    {}

    /// Open config dialog
    QWidget* config(QWidget* win) override { return new mpcConfigFrame(win); }
};

/*****************************************************************/

class mpcBackend : public input
{
    friend class mpcConfigFrame;

private:
#ifdef MPCDEC_SV8
    mpc_demux* m_demux;
#else
    mpc_decoder m_decoder;
#endif
    mpc_streaminfo m_si;
    mpc_reader m_mpcReader;

    MPC_SAMPLE_FORMAT m_buffer[MPC_DECODER_BUFFER_LENGTH];
    unsigned int m_bufIndex;
    unsigned int m_bufLen;

    QFile m_file;

    mpcConfig m_config;

private:
    static mpc_int32_t read_func(DATAPARM, void *ptr, mpc_int32_t size);
    static mpc_bool_t  seek_func(DATAPARM, mpc_int32_t offset);
    static mpc_int32_t tell_func(DATAPARM);
    static mpc_int32_t get_size_func(DATAPARM);
    static mpc_bool_t canseek_func(DATAPARM);

private:
    mpcBackend(const QString& fileName);

public:
    ~mpcBackend();

    static const char name[];

    /// Factory method
    static input* factory(const QString& fileName) { return new mpcBackend(fileName); }
    static inputConfig* cFactory();

    /// Get supported extension
    static QStringList ext();

    /// Seek support
    bool seekable() const override { return true; }

    /// Seek specified position
    bool seek(int pos) override;

    /// Get samplerate
    unsigned int samplerate() const override { return m_si.sample_freq; }

    /// Get channels
    unsigned int channels() const override { return m_si.channels; }

    /// Get precision
    sample_t precision() const override
    {
#ifdef MPC_FIXED_POINT
        return sample_t::SAMPLE_FIXED;
#else
        return sample_t::SAMPLE_FLOAT;
#endif
    }

#ifdef MPC_FIXED_POINT
    /// Get fractional scale for fixed point types
    unsigned int fract() const override { return MPC_FIXED_POINT_SCALE_SHIFT; }
#endif
    /// Callback function
    size_t fillBuffer(void* buffer, const size_t bufferSize) override;

    /// Gapless support
    bool gapless() const override { return !songLoaded().isNull() ? m_si.is_true_gapless : true; }
};

#endif
