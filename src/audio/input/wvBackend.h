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
#include "inputConfig.h"

/*****************************************************************/

#include "configFrame.h"

class wvConfigFrame : public configFrame
{
private:
    wvConfigFrame() {}
    wvConfigFrame(const wvConfigFrame&) = delete;
    wvConfigFrame& operator=(const wvConfigFrame&) = delete;

public:
    wvConfigFrame(QWidget* win);
    ~wvConfigFrame() override = default;
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
    WavpackContext *m_wvContext;
    int *m_decodeBuf;
    int m_bufOffset;
    int m_bufSize;
    int m_bps;
    int m_channels;
    sample_t m_precision;

    wvConfig m_config;

private:
    void copyBuffer(char* dest, const int* src, size_t length);
    void getApeTag(const char* tag, metaData::mpris_t meta);
    void getId3Tag(const char* tag, metaData::mpris_t meta);

private:
    wvBackend(const QString& fileName);

public:
    ~wvBackend() override;

    static const char name[];

    /// Factory method
    static input* factory(const QString& fileName) { return new wvBackend(fileName); }
    static inputConfig* cFactory();

    /// Get supported extension
    static QStringList ext();

    /// Seek support
    bool seekable() const override { return true; }

    /// Seek specified position
    bool seek(double pos) override;

    /// Get samplerate
    unsigned int samplerate() const override
    {
        return WavpackGetSampleRate(m_wvContext);
    }

    /// Get channels
    unsigned int channels() const override
    {
        return WavpackGetReducedChannels(m_wvContext);
    }

    /// Get precision
    sample_t precision() const override { return m_precision; }

    /// Get fractional scale for fixed point types
    unsigned int fract() const override { return (m_bps<<3)-1; }

    /// Callback function
    size_t fillBuffer(void* buffer, size_t bufferSize) override;

    /// Gapless support
    bool gapless() const override { return true; };
};

#endif
