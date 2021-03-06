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

#ifndef SID_H
#define SID_H

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <sidplayfp/sidplayfp.h>
#include <sidplayfp/SidDatabase.h>
#include <sidplayfp/SidTune.h>
#include <sidplayfp/SidConfig.h>
#include <sidplayfp/SidTuneInfo.h>

#include "inputBackend.h"

#ifdef HAVE_STILVIEW
#  include <stilview/stil.h>
#endif

#if (LIBSIDPLAYFP_VERSION_MAJ > 1) || (LIBSIDPLAYFP_VERSION_MAJ == 1 && LIBSIDPLAYFP_VERSION_MIN > 7)
#  define ENABLE_3SID
#endif


/*****************************************************************/

typedef struct
{
    QString engine;
    int samplerate;
    int channels;
    SidConfig::sampling_method_t samplingMethod;
    bool fastSampling;
    int bias;
    int filter6581Curve;
    int filter8580Curve;
    SidConfig::c64_model_t c64Model;
    bool forceC64Model;
    SidConfig::sid_model_t sidModel;
    bool forceSidModel;
    bool filter;
    QString hvscPath;
    int secondSidAddress;
    int thirdSidAddress;
    QString kernalPath;
    QString basicPath;
    QString chargenPath;
} sidConfig_t;

/*****************************************************************/

#include "configFrame.h"

class QLineEdit;

class sidConfig : public configFrame
{
private:
    sidConfig() {}
    sidConfig(const sidConfig&);
    sidConfig& operator=(const sidConfig&);

private:
    bool checkPath(const QString& path);

public:
    sidConfig(QWidget* win);
    virtual ~sidConfig() {}
};

/*****************************************************************/

class sidBackend : public inputBackend
{
    friend class sidConfig;

private:
    static sidConfig_t _settings;

private:
    sidplayfp *_sidplayfp;

    SidTune *_tune;

    STIL *_stil;

    SidDatabase *_db;
    unsigned int _length;
    char _md5[SidTune::MD5_LENGTH+1];
    bool newSonglengthDB;

private:
    sidBackend();

    void loadTune(const int num);

    void openHvsc(const QString& hvscPath);

    const unsigned char* loadRom(const QString& romPath);

public:
    ~sidBackend();

    static const char name[];

    /// Factory method
    static inputBackend* factory() { return new sidBackend(); }

    /// Get supported extension
    static QStringList ext();

    /// Open file
    bool open(const QString& fileName) override;

    /// Close file
    void close() override;

    /// Rewind to start
    bool rewind() override;

    /// Number of subtunes
    unsigned int subtunes() const override { return _tune ? _tune->getInfo()->songs() : 0; }

    /// Current subtune
    unsigned int subtune() const override { return _tune ? _tune->getInfo()->currentSong() : 0; }

    /// Change subtune
    bool subtune(const unsigned int i) override;

    /// Get samplerate
    unsigned int samplerate() const override { return _tune?_settings.samplerate:0; }

    /// Get channels
    unsigned int channels() const override { return _tune?_settings.channels:0; }

    /// Get precision
    sample_t precision() const override { return sample_t::S16; }

    /// Callback function
    size_t fillBuffer(void* buffer, const size_t bufferSize, const unsigned int milliSeconds) override;

    /// Open config dialog
    QWidget* config(QWidget* win) override { return new sidConfig(win); }

    /// Get Music directory
    const QString getMusicDir(void) const override { return _settings.hvscPath; }

    void loadSettings() override;

    void saveSettings() override;
};

#endif
