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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef GAME_H
#define GAME_H

#include "libs/gme/Music_Emu.h"

#include "inputBackend.h"

#ifdef HAVE_STILVIEW
#  include <stilview/stil.h>
#else
#  include "stilview/stil.h"
#endif

/*****************************************************************/

typedef struct
{
    int samplerate;
    QString asmaPath;
} gmeConfig_t;

/*****************************************************************/

#include "configFrame.h"

class gmeConfig : public configFrame
{
    Q_OBJECT

private:
    gmeConfig() {}
    gmeConfig(const gmeConfig&);
    gmeConfig& operator=(const gmeConfig&);

public:
    gmeConfig(QWidget* win);
    virtual ~gmeConfig() {}

public:
    void onCmdSamplerate(int val);
    void onCmdAsma();
};

/*****************************************************************/

class gmeBackend : public inputBackend
{
    friend class gmeConfig;

private:
    static gmeConfig_t _settings;

private:
    Music_Emu *_emu;
    bool _hasStilInfo;

    STIL *_stil;

private:
    gmeBackend();

    void openAsma(const QString& asmaPath);

    void getInfo();

    bool check(const char* error);

public:
    ~gmeBackend();

    static const char name[];

    /// Factory method
    static input* factory() { return new gmeBackend(); }

    /// Check if we support ext
    static bool supports(const QString& fileName);

    /// Get supported extension
    QStringList ext() const override;

    /// Open file
    bool open(const QString& fileName) override;

    /// Close file
    void close() override;

    /// Rewind to start
    bool rewind() override;

    /// Get number of subtunes
    unsigned int subtunes() const override { return _emu ? _emu->track_count() : 0; }

    /// Get current subtune
    unsigned int subtune() const override { return _emu ? _emu->current_track()+1 : 0; }

    /// Change subtune
    bool subtune(const unsigned int i) override;

    /// Get samplerate
    unsigned int samplerate() const override { return _settings.samplerate; }

    /// Get channels
    unsigned int channels() const override { return 2; }

    /// Get precision
    sample_t precision() const override { return sample_t::S16; }

    /// Callback function
    size_t fillBuffer(void* buffer, const size_t bufferSize, const unsigned int seconds) override;

    /// Open config dialog
    QWidget* config(QWidget* win) override { return new gmeConfig(win); }

    /// Get Music directory
    const QString getMusicDir() const { return _settings.asmaPath; }

    void loadSettings();

    void saveSettings();
};

#endif
