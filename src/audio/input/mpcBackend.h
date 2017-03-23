/*
 *  Copyright (C) 2006-2017 Leandro Nini
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

#ifdef HAVE_MPC_MPCDEC_H
#  include <mpc/mpcdec.h>
#  define DATAPARM mpc_reader *p_reader
#  define DATAFILE (p_reader->data)
#  define SV8
#else
#  include <mpcdec/mpcdec.h>
#  define DATAPARM void *data
#  define DATAFILE data
#endif

#include "inputBackend.h"

#include <QFile>

/*****************************************************************/

#include "configFrame.h"

class mpcConfig : public configFrame
{
    Q_OBJECT

private:
    mpcConfig() {}
    mpcConfig(const mpcConfig&);
    mpcConfig& operator=(const mpcConfig&);

public:
    mpcConfig(QWidget* win);
    virtual ~mpcConfig() {}
};

/*****************************************************************/

class mpcBackend : public inputBackend
{
    friend class mpcConfig;

private:
#ifdef SV8
    mpc_demux* _demux;
#else
    mpc_decoder _decoder;
#endif
    mpc_streaminfo _si;
    mpc_reader _mpcReader;

    MPC_SAMPLE_FORMAT _buffer[MPC_DECODER_BUFFER_LENGTH];
    unsigned int _bufIndex;
    unsigned int _bufLen;

    QFile _file;

private:
    static mpc_int32_t read_func(DATAPARM, void *ptr, mpc_int32_t size);
    static mpc_bool_t  seek_func(DATAPARM, mpc_int32_t offset);
    static mpc_int32_t tell_func(DATAPARM);
    static mpc_int32_t get_size_func(DATAPARM);
    static mpc_bool_t canseek_func(DATAPARM);

private:
    mpcBackend();

public:
    ~mpcBackend();

    static const char name[];

    /// Factory method
    static input* factory() { return new mpcBackend(); }

    /// Check if we support ext
    static bool supports(const QString& fileName);

    /// Get supported extension
    QStringList ext() const;

    /// Open file
    bool open(const QString& fileName) override;

    /// Close file
    void close() override;

    /// Rewind to start
    bool rewind() override;

    /// Get samplerate
    unsigned int samplerate() const override { return _si.sample_freq; }

    /// Get channels
    unsigned int channels() const override { return _si.channels; }

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
    size_t fillBuffer(void* buffer, const size_t bufferSize, const unsigned int seconds) override;

    /// Gapless support
    bool gapless() const override { return !songLoaded().isNull() ? _si.is_true_gapless : true; }

    /// Open config dialog
    QWidget* config(QWidget* win) override { return new mpcConfig(win); }
};

#endif
