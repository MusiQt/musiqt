/*
 *  Copyright (C) 2007-2021 Leandro Nini
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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef WV_H
#define WV_H

extern "C" {
#include <wavpack/wavpack.h>
}

#include "input.h"

/*****************************************************************/

#include "configFrame.h"

class wvConfigFrame : public configFrame
{
private:
    wvConfigFrame() {}
    wvConfigFrame(const wvConfigFrame&);
    wvConfigFrame& operator=(const wvConfigFrame&);

public:
    wvConfigFrame(QWidget* win);
    virtual ~wvConfigFrame() {}
};

/*****************************************************************/

class wvConfig : public inputConfig
{
public:
    wvConfig(const char name[], const unsigned char* iconType, unsigned int iconLen) :
        inputConfig(name, iconType, iconLen)
    {}

    /// Open config dialog
    QWidget* config(QWidget* win) override { return new wvConfigFrame(win); }
};

/*****************************************************************/

class wvBackend : public input
{
private:
    WavpackContext *_wvContext;
    int *_decodeBuf;
    int _bufOffset;
    int _bufSize;
    int _bps;
    int _channels;
    sample_t _precision;

    wvConfig m_config;

private:
    void copyBuffer(char* dest, const int* src, size_t length);
    void getApeTag(const char* tag, metaData::mpris_t meta);
    void getId3Tag(const char* tag, metaData::mpris_t meta);

private:
    wvBackend();

public:
    ~wvBackend();

    static const char name[];

    /// Factory method
    static input* factory() { return new wvBackend(); }
    static inputConfig* cFactory();

    /// Get supported extension
    static QStringList ext();

    /// Open file
    bool open(const QString& fileName) override;

    /// Close file
    void close();

    /// Seek support
    bool seekable() const override { return true; }

    /// Seek specified position
    bool seek(int pos) override;

    /// Get samplerate
    unsigned int samplerate() const override
    {
        return _wvContext != nullptr
            ? WavpackGetSampleRate(_wvContext)
            : 0;
    }

    /// Get channels
    unsigned int channels() const override
    {
        return _wvContext != nullptr
            ? WavpackGetReducedChannels(_wvContext)
            : 0;
    }

    /// Get precision
    sample_t precision() const override { return _precision; }

    /// Get fractional scale for fixed point types
    unsigned int fract() const override { return (_bps<<3)-1; }

    /// Callback function
    size_t fillBuffer(void* buffer, size_t bufferSize) override;

    /// Gapless support
    bool gapless() const override { return true; };
};

#endif
