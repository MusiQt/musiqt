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

#include "input.h"

#ifdef HAVE_STILVIEW
#  include <stilview/stil.h>
#endif

/*****************************************************************/

typedef struct
{
    int samplerate;
    bool equalizer;
    double treble_dB;
    double bass_freq;
    QString asmaPath;
} gmeConfig_t;

/*****************************************************************/

#include "configFrame.h"

class QLineEdit;

class gmeConfigFrame : public configFrame
{
private:
    gmeConfigFrame() {}
    gmeConfigFrame(const gmeConfigFrame&);
    gmeConfigFrame& operator=(const gmeConfigFrame&);

public:
    gmeConfigFrame(QWidget* win);
    virtual ~gmeConfigFrame() {}
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
    Music_Emu *_emu;
    int _currentTrack;
#ifdef HAVE_STILVIEW
    STIL *_stil;
#endif
    static QStringList _ext;

    gmeConfig m_config;

private:
    gmeBackend();

    void openAsma(const QString& asmaPath);

    void getInfo();

    bool checkRetCode(const char* error);

public:
    ~gmeBackend();

    static const char name[];

    static bool init();

    /// Factory method
    static input* factory() { return new gmeBackend(); }
    static inputConfig* cFactory();

    /// Get supported extension
    static QStringList ext();

    /// Open file
    bool open(const QString& fileName) override;

    /// Rewind to start
    bool rewind() override;

    /// Get number of subtunes
    unsigned int subtunes() const override { return _emu ? gme_track_count(_emu) : 0; }

    /// Get current subtune
    unsigned int subtune() const override { return _currentTrack+1; }

    /// Change subtune
    bool subtune(const unsigned int i) override;

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
