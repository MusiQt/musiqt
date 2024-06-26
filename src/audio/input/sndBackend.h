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

#ifndef SND_H
#define SND_H

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#ifdef ENABLE_SNDFILE_WINDOWS_PROTOTYPES
#  include <windows.h>
#endif

#include <sndfile.h>

#include "input.h"
#include "inputConfig.h"

/*****************************************************************/

#include "configFrame.h"

class sndConfigFrame : public configFrame
{
private:
    sndConfigFrame() {}
    sndConfigFrame(const sndConfigFrame&) = delete;
    sndConfigFrame& operator=(const sndConfigFrame&) = delete;

public:
    sndConfigFrame(QWidget* win);
    ~sndConfigFrame() override = default;
};

/*****************************************************************/

class sndConfig : public inputConfig
{
public:
    sndConfig(const char name[]) :
        inputConfig(name)
    {}

    /// Open config dialog
    QWidget* config(QWidget* win) override { return new sndConfigFrame(win); }
};

/*****************************************************************/

class sndBackend : public input
{
private:
    SNDFILE *m_sf;
    SF_INFO m_si;

    static QStringList m_ext;

    sndConfig m_config;

private:
    sndBackend(const QString& fileName);

public:
    ~sndBackend() override;

    static const char name[];

    static bool init();

    /// Factory method
    static input* factory(const QString& fileName) { return new sndBackend(fileName); }
    static inputConfig* cFactory();

    /// Get supported extension
    static QStringList ext() { return m_ext; }

    /// Seek support
    bool seekable() const override { return true; }

    /// Seek specified position
    bool seek(double pos) override;

    /// Get samplerate
    unsigned int samplerate() const override { return m_sf ? m_si.samplerate : 0; }

    /// Get channels
    unsigned int channels() const override { return m_sf ? m_si.channels : 0; }

    /// Get precision
    sample_t precision() const override { return sample_t::S16; }

    /// Callback function
    size_t fillBuffer(void* buffer, const size_t bufferSize) override;

    /// Gapless support
    bool gapless() const override { return true; }
};

#endif
