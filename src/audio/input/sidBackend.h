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

#include "input.h"
#include "inputConfig.h"

#ifdef HAVE_STILVIEW
#  include <stilview/stil.h>
#endif

#if (LIBSIDPLAYFP_VERSION_MAJ > 1) || (LIBSIDPLAYFP_VERSION_MAJ == 1 && LIBSIDPLAYFP_VERSION_MIN > 7)
#  define ENABLE_3SID
#endif


/*****************************************************************/

struct sidConfig_t
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
};

/*****************************************************************/

#include "configFrame.h"

class QLineEdit;

class sidConfigFrame : public configFrame
{
private:
    sidConfigFrame() {}
    sidConfigFrame(const sidConfigFrame&);
    sidConfigFrame& operator=(const sidConfigFrame&);

private:
    bool checkPath(const QString& path);

public:
    sidConfigFrame(QWidget* win);
    ~sidConfigFrame() override {}
};

/*****************************************************************/

class sidConfig : public inputConfig
{
    friend class sidConfigFrame;

private:
    static sidConfig_t m_settings;

public:
    sidConfig(const char name[], const unsigned char* iconType, unsigned int iconLen) :
        inputConfig(name, iconType, iconLen)
    {
        loadSettings();
    }

    /// Get Music directory
    const QString getMusicDir() const override { return hvscPath(); }

    /// Open config dialog
    QWidget* config(QWidget* win) override { return new sidConfigFrame(win); }

    QString engine() const { return m_settings.engine; }

    int samplerate() const { return m_settings.samplerate; }

    int channels() const { return m_settings.channels; }

    SidConfig::sampling_method_t samplingMethod() const { return m_settings.samplingMethod; }

    bool fastSampling() const { return m_settings.fastSampling; }

    int bias() const { return m_settings.bias; }

    int filter6581Curve() const { return m_settings.filter6581Curve; }

    int filter8580Curve() const { return m_settings.filter8580Curve; }

    SidConfig::c64_model_t c64Model() const { return m_settings.c64Model; }

    bool forceC64Model() const { return m_settings.forceC64Model; }

    SidConfig::sid_model_t sidModel() const { return m_settings.sidModel; }

    bool forceSidModel() const { return m_settings.forceSidModel; }

    bool filter() const { return m_settings.filter; }

    QString hvscPath() const { return m_settings.hvscPath; }

    int secondSidAddress() const { return m_settings.secondSidAddress; }

    int thirdSidAddress() const { return m_settings.thirdSidAddress; }

    QString kernalPath() const { return m_settings.kernalPath; }

    QString basicPath() const { return m_settings.basicPath; }

    QString chargenPath() const { return m_settings.chargenPath; }

    void loadSettings() override;

    void saveSettings() override;
};

/*****************************************************************/

class sidBackend : public input
{
private:
    sidplayfp *m_sidplayfp;

    SidTune *m_tune;

#ifdef HAVE_STILVIEW
    STIL *m_stil;
#endif

    SidDatabase *m_db;
    bool m_newSonglengthDB;

    sidConfig m_config;

private:
    sidBackend(const QString& fileName);

    void createEmu();

    void deleteEmu();

    bool loadTune(int num);

    void openHvsc(const QString& hvscPath);

    const unsigned char* loadRom(const QString& romPath);

    void getInfo(const SidTuneInfo* tuneInfo) noexcept;

    void loadWDS(const QString& musFileName, const char* ext);

public:
    ~sidBackend() override;

    static const char name[];

    /// Factory method
    static input* factory(const QString& fileName) { return new sidBackend(fileName); }
    static inputConfig* cFactory();

    /// Get supported extension
    static QStringList ext();

    /// Rewind to start
    bool rewind() override;

    /// Number of subtunes
    unsigned int subtunes() const override { return m_tune->getInfo()->songs(); }

    /// Current subtune
    unsigned int subtune() const override { return m_tune->getInfo()->currentSong(); }

    /// Change subtune
    bool subtune(unsigned int i) override;

    /// Get samplerate
    unsigned int samplerate() const override { return m_config.samplerate(); }

    /// Get channels
    unsigned int channels() const override { return m_config.channels(); }

    /// Get precision
    sample_t precision() const override { return sample_t::S16; }

    /// Get max play time in milliseconds, 0 if none
    unsigned int maxPlayTime() const override { return songDuration(); }

    /// Callback function
    size_t fillBuffer(void* buffer, const size_t bufferSize) override;
};

#endif
