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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef GAME_H
#define GAME_H

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <gme/gme.h>
#undef check // conflicts with Qt6

#include "input.h"
#include "inputConfig.h"

#ifdef HAVE_STILVIEW
#  include <stilview/stil.h>
#endif

/*****************************************************************/

struct gmeConfig_t
{
    int samplerate;
    bool equalizer;
    double treble_dB;
    double bass_freq;
    QString asmaPath;
};

/*****************************************************************/

#include "configFrame.h"

class QLineEdit;

class gmeConfigFrame : public configFrame
{
private:
    gmeConfigFrame() {}
    gmeConfigFrame(const gmeConfigFrame&) = delete;
    gmeConfigFrame& operator=(const gmeConfigFrame&) = delete;

public:
    gmeConfigFrame(QWidget* win);
    ~gmeConfigFrame() override = default;
};

/*****************************************************************/

class gmeConfig : public inputConfig
{
    friend class gmeConfigFrame;

private:
    static gmeConfig_t m_settings;

public:
    gmeConfig(const char name[]) :
        inputConfig(name)
    {
        loadSettings();
    }

    void loadSettings() override;

    void saveSettings() override;

    /// Get Music directory
    const QString getMusicDir() const override { return asmaPath(); }

    /// Open config dialog
    QWidget* config(QWidget* win) override { return new gmeConfigFrame(win); }

    unsigned int samplerate() const { return m_settings.samplerate; }

    bool equalizer() const { return m_settings.equalizer; }

    double treble_dB() const { return m_settings.treble_dB; }

    double bass_freq() const { return m_settings.bass_freq; }

    QString asmaPath() const { return m_settings.asmaPath; }
};

/*****************************************************************/

class gmeBackend : public input
{
private:
    Music_Emu *m_emu;
    int m_currentTrack;
#ifdef HAVE_STILVIEW
    STIL *m_stil;
#endif
    static QStringList m_ext;

    gmeConfig m_config;

private:
    gmeBackend(const QString& fileName);

    void openAsma(const QString& asmaPath);

    void getInfo();

    void checkRetCode(const char* error);

public:
    ~gmeBackend() override;

    static const char name[];

    static bool init();

    /// Factory method
    static input* factory(const QString& fileName) { return new gmeBackend(fileName); }
    static inputConfig* cFactory();

    /// Get supported extension
    static QStringList ext();

    /// Rewind to start
    bool rewind() override;

    /// Get number of subtunes
    unsigned int subtunes() const override { return gme_track_count(m_emu); }

    /// Get current subtune
    unsigned int subtune() const override { return m_currentTrack+1; }

    /// Change subtune
    bool subtune(unsigned int i) override;

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
