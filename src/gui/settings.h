/*
 *  Copyright (C) 2008-2025 Leandro Nini
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

#ifndef SETTINGS_H
#define SETTINGS_H

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <QDialog>
#include <QObject>
#include <QLabel>

namespace config
{
constexpr const char* GENERAL_SUBTUNES   = "General Settings/play subtunes";
constexpr const char* GENERAL_REPLAYGAIN = "General Settings/Replaygain";
constexpr const char* GENERAL_RG_MODE    = "General Settings/Replaygain mode";
constexpr const char* GENERAL_BAUERDSP   = "General Settings/Bauer DSP";
constexpr const char* GENERAL_ICONTHEME  = "General Settings/Theme icons";
constexpr const char* GENERAL_POS        = "General Settings/pos";
constexpr const char* GENERAL_SIZE       = "General Settings/size";
constexpr const char* GENERAL_PLMODE     = "General Settings/playlist mode";
constexpr const char* GENERAL_FILE       = "General Settings/file";

constexpr const char* AUDIO_CARD         = "Audio Settings/card";
constexpr const char* AUDIO_BITS         = "Audio Settings/bits";
constexpr const char* AUDIO_VOLUME       = "Audio Settings/volume";
constexpr const char* AUDIO_BUFFERLEN    = "Audio Settings/buffer length";

constexpr const char* LASTFM_USERNAME    = "Last.fm Settings/User Name";
constexpr const char* LASTFM_SESSIONKEY  = "Last.fm Settings/Session Key";
constexpr const char* LASTFM_SCROBBLING  = "Last.fm Settings/scrobbling";
};

/*****************************************************************/

class inputConfig;

class settingsWindow : public QDialog
{
private:
    QList<inputConfig*> m_inputConfigs;

private:
    settingsWindow() {}
    settingsWindow(const settingsWindow&) = delete;
    settingsWindow& operator=(const settingsWindow&) = delete;

protected:
    bool event(QEvent *e) override;

public:
    settingsWindow(QWidget* win, const QString& bkName);
    ~settingsWindow() override = default;
};

/*****************************************************************/

#define SETTINGS settings::instance()

class QSettings;

class settings
{
    friend class settingsWindow;
    friend class audioConfig;

public:
    enum class rg_t
    {
        Album,
        Track
    };

private:
    QString      m_card;
    unsigned int m_bits;
    unsigned int m_bufLen;

    bool         m_subtunes;
    bool         m_bs2b;
    bool         m_themeIcons;
    bool         m_replayGain;
    rg_t         m_replayGainMode;

protected:
    settings() {}
    settings(const settings&);
    settings& operator= (const settings&);
    ~settings() = default;

public:
    /// Get singleton instance
    static settings* instance();

    /// Load setting
    void load(const QSettings& appSettings);

    /// Save settings
    void save(QSettings& appSettings);

    // Get program settings

    /// Soundcard
    const QString& card() const { return m_card; }

    /// Play subtunes
    bool subtunes() const { return m_subtunes; }

    /// Replay Gain
    bool replayGain() const { return m_replayGain; }

    /// Replay Gain Mode
    rg_t replayGainMode() const { return m_replayGainMode; }

    /// Bauer stereophonic-to-binaural DSP
    bool bs2b() const { return m_bs2b; }

    /// System theme icons
    bool themeIcons() const { return m_themeIcons; }

    /// Default bittdepth
    unsigned int bits() const { return m_bits; }

    /// Default buffer length
    unsigned int bufLen() const { return m_bufLen; }
};

#endif
