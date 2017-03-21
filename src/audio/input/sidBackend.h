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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef SID_H
#define SID_H

#include <sidplayfp/sidplayfp.h>
#include <sidplayfp/SidDatabase.h>
#include <sidplayfp/SidTune.h>
#include <sidplayfp/SidConfig.h>
#include <sidplayfp/SidTuneInfo.h>

#include "inputBackend.h"

#ifdef HAVE_STILVIEW
#  include <stilview/stil.h>
#else
#  include "stilview/stil.h"
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
#ifdef ENABLE_3SID
    int thirdSidAddress;
#endif
    QString kernalPath;
    QString basicPath;
    QString chargenPath;
} sidConfig_t;

/*****************************************************************/

#include "configFrame.h"

class sidConfig : public configFrame
{
    Q_OBJECT

private:
    QVBoxLayout* _biasFrame;
    QVBoxLayout* _filterCurveFrame;

private:
    sidConfig() {}
    sidConfig(const sidConfig&);
    sidConfig& operator=(const sidConfig&);

public:
    sidConfig(QWidget* win);
    virtual ~sidConfig() {}

private slots:
    void onCmdFrequency(int val);
    void onCmdChannels(int val);
    void onCmdSampling(int val);
    void onCmdEngine(int val);
    void onCmdClock(int val);
    void onCmdModel(int val);
    void onCmdAddress2(int val);
#ifdef ENABLE_3SID
    void onCmdAddress3(int val);
#endif
    void onCmdHvsc();
    void onCmdRom(int val);
    void onCmdForceSidModel(bool val);
    void onCmdForceC64Model(bool val);
    void onCmdFastSampling(bool val);
    void onCmdFilter(bool val);
};

/*****************************************************************/

class sidBackend : public inputBackend
{
    friend class sidConfig;

private:
    static sidConfig_t _settings;

private:
    sidplayfp *_sidplayfp;

    STIL *_stil;

    SidDatabase	*_db;
    unsigned int _length;
    char _md5[SidTune::MD5_LENGTH+1];

    SidTune *_tune;

private:
    sidBackend();

    void loadTune(const int num);

    void openHvsc(const QString& hvscPath);

    const unsigned char* loadRom(const QString& romPath);

public:
    ~sidBackend();

    static const char name[];

    /// Factory method
    static input* factory() { return new sidBackend(); }

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

    /// Number of subtunes
    unsigned int subtunes() const override { return _tune?_tune->getInfo()->songs():0; }

    /// Current subtune
    unsigned int subtune() const override { return _tune?_tune->getInfo()->currentSong():0; }

    /// Change subtune
    bool subtune(const unsigned int i) override;

    /// Get samplerate
    unsigned int samplerate() const override { return _tune?_settings.samplerate:0; }

    /// Get channels
    unsigned int channels() const override { return _tune?_settings.channels:0; }

    /// Get precision
    sample_t precision() const override { return S16; }

    /// Callback function
    size_t fillBuffer(void* buffer, const size_t bufferSize, const unsigned int seconds) override;

    /// Open config dialog
    QWidget* config(QWidget* win) override { return new sidConfig(win); }

    /// Get Music directory
    const QString getMusicDir(void) override { return _settings.hvscPath; }

    void loadSettings();

    void saveSettings();
};

#endif
