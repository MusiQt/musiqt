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

#ifndef HVL_H
#define HVL_H

extern "C"{
#include "libs/hvl_replay/hvl_replay.h"
}

#include "input.h"
#include "inputConfig.h"

/*****************************************************************/

struct hvlConfig_t
{
    unsigned int samplerate;
};

/*****************************************************************/

#include "configFrame.h"

class hvlConfigFrame final : public configFrame
{
    Q_OBJECT

private:
    hvlConfigFrame() {}
    hvlConfigFrame(const hvlConfigFrame&);
    hvlConfigFrame& operator=(const hvlConfigFrame&);

public:
    explicit hvlConfigFrame(QWidget* win);
    ~hvlConfigFrame() override = default;
};

/*****************************************************************/

class hvlConfig final : public inputConfig
{
    friend class hvlConfigFrame;

private:
    hvlConfig_t m_settings;

private:
    hvlConfig(const char name[], const unsigned char* iconType, unsigned int iconLen) :
        inputConfig(name, iconType, iconLen)
    {
        loadSettings();
    }

    hvlConfig(const hvlConfig&) = delete;
    hvlConfig& operator=(hvlConfig) = delete;

public:
    static hvlConfig& instance();

    void loadSettings() override;

    void saveSettings() override;

    /// Open config dialog
    QWidget* config(QWidget* win) override { return new hvlConfigFrame(win); }

    unsigned int samplerate() const { return m_settings.samplerate; }
};

/*****************************************************************/

class hvlBackend final : public input
{
    friend class hvlConfigFrame;

private:
    struct hvl_tune *m_tune;

    int m_subtune;

    int m_left;
    unsigned int m_size;
    char *m_buffer;

private:
    explicit hvlBackend(const QString& fileName);

    hvlBackend(const hvlBackend&) = delete;
    hvlBackend& operator=(hvlBackend) = delete;

public:
    ~hvlBackend() override;

    static const char name[];

    /// Factory methods
    static input* factory(const QString& fileName) { return new hvlBackend(fileName); }
    static inputConfig& cfgFactory();

    /// Get supported extension
    static QStringList ext();

    /// Rewind to start
    bool rewind() override;

    /// Get number of subtunes
    unsigned int subtunes() const override { return m_tune->ht_SubsongNr; }

    /// Get current subtune
    unsigned int subtune() const override { return m_tune->ht_SongNum; }

    /// Change subtune
    bool subtune(unsigned int i) override;

    /// Get samplerate
    unsigned int samplerate() const override { return hvlConfig::instance().samplerate(); }

    /// Get channels
    unsigned int channels() const override { return 2; }

    /// Get precision
    sample_t precision() const override { return sample_t::S16; }

    /// Callback function
    size_t fillBuffer(void* buffer, const size_t bufferSize) override;
};

#endif
