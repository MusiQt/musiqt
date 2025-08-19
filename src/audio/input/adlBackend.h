/*
 *  Copyright (C) 2006-2025 Leandro Nini
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

#ifndef ADL_H
#define ADL_H

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <adlmidi.h>

#include "input.h"
#include "inputConfig.h"

#include <vector>

/*****************************************************************/

struct adlConfig_t
{
    int samplerate;
    QString woplPath;
};

/*****************************************************************/

#include "configFrame.h"

class QLineEdit;

class adlConfigFrame : public configFrame
{
private:
    adlConfigFrame() {}
    adlConfigFrame(const adlConfigFrame&) = delete;
    adlConfigFrame& operator=(const adlConfigFrame&) = delete;

public:
    adlConfigFrame(QWidget* win);
    ~adlConfigFrame() override = default;
};

/*****************************************************************/

class adlConfig : public inputConfig
{
    friend class adlConfigFrame;

private:
    static adlConfig_t m_settings;

public:
    adlConfig(const char name[]) :
        inputConfig(name)
    {
        loadSettings();
    }

    void loadSettings() override;

    void saveSettings() override;

    /// Open config dialog
    QWidget* config(QWidget* win) override { return new adlConfigFrame(win); }

    unsigned int samplerate() const { return m_settings.samplerate; }

    QString woplPath() const { return m_settings.woplPath; }
};

/*****************************************************************/

class adlBackend : public input
{
private:
    struct ADL_MIDIPlayer *m_player;
    struct ADLMIDI_AudioFormat m_format;

    int m_currentTrack;
    int m_subtunes;
    static QStringList m_ext;

    std::vector<ADL_UInt8> m_buffer;

    adlConfig m_config;

private:
    adlBackend(const QString& fileName);

public:
    ~adlBackend() override;

    static const char name[];

    static bool init();

    /// Factory method
    static input* factory(const QString& fileName) { return new adlBackend(fileName); }
    static inputConfig* cFactory();

    /// Get supported extension
    static QStringList ext();

    /// Rewind to start
    bool rewind() override;

    /// Get number of subtunes
    unsigned int subtunes() const override { return m_subtunes+1; }

    /// Get current subtune
    unsigned int subtune() const override { return m_currentTrack+1; }

    /// Change subtune
    bool subtune(unsigned int i) override;

    /// Seek support
    bool seekable() const override { return true; }

    /// Seek specified position
    bool seek(double pos) override;

    /// Get samplerate
    unsigned int samplerate() const override { return m_config.samplerate(); }

    /// Get channels
    unsigned int channels() const override { return 2; }

    /// Get precision
    sample_t precision() const override { return sample_t::S16; }

    /// Callback function
    size_t fillBuffer(void* buffer, const size_t bufferSize) override;
};

#endif
