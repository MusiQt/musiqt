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

#ifndef OPUS_BACKEND_H
#define OPUS_BACKEND_H

#include "inputBackend.h"

#include <opusfile.h>

#include <QFile>

/*****************************************************************/

#include "configFrame.h"

class opusConfigFrame : public configFrame
{
private:
    opusConfigFrame() {}
    opusConfigFrame(const opusConfigFrame&);
    opusConfigFrame& operator=(const opusConfigFrame&);

public:
    opusConfigFrame(QWidget* win);
    virtual ~opusConfigFrame() {}
};

/*****************************************************************/

class opusConfig : public inputConfig
{
    friend class opusConfigFrame;

public:
    opusConfig(const char name[], const unsigned char* iconType, unsigned int iconLen) :
        inputConfig(name, iconType, iconLen)
    {}

    /// Open config dialog
    QWidget* config(QWidget* win) override { return new opusConfigFrame(win); }
};

/*****************************************************************/

class opusBackend : public inputBackend
{
private:
    OggOpusFile *_of;

    QFile _file;

    opusConfig m_config;

private:
    static OpusFileCallbacks opus_callbacks;

    static int read_func(void *_stream, unsigned char *_ptr, int _nbytes);
    static int seek_func(void *_stream, opus_int64 _offset, int _whence);
    static int close_func(void *_stream);
    static opus_int64 tell_func(void *_stream);

private:
    opusBackend();

public:
    ~opusBackend();

    static const char name[];

    /// Factory method
    static inputBackend* factory() { return new opusBackend(); }

    /// Get supported extension
    static QStringList ext();

    /// Open file
    bool open(const QString& fileName) override;

    /// Close file
    void close() override;

    /// Seek support
    bool seekable() const override { return true; }

    /// Seek specified position
    bool seek(int pos) override;

    /// Get samplerate
    unsigned int samplerate() const override { return 48000; }

    /// Get channels
    unsigned int channels() const override { return 2; }

    /// Get precision
    sample_t precision() const override { return sample_t::S16; }

    /// Callback function
    size_t fillBuffer(void* buffer, const size_t bufferSize);

    /// Gapless support
    bool gapless() const override { return true; };

    // TODO remove

    /// Open config dialog
    QWidget* config(QWidget* win) override { return m_config.config(win); }
};

#endif
