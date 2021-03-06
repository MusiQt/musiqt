/*
 *  Copyright (C) 2008-2021 Leandro Nini
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

class inputConfig;

class settingsWindow : public QDialog
{
private:
    QLabel *colorLabel;

    QList<inputConfig*> inputConfigs;

private:
    settingsWindow() {}
    settingsWindow(const settingsWindow&);
    settingsWindow& operator=(const settingsWindow&);

protected:
    bool event(QEvent *e) override;

public:
    settingsWindow(QWidget* win);
    virtual ~settingsWindow() {}
};

/*****************************************************************/

#define SETTINGS settings::instance()

class QSettings;

class settings
{
    friend class settingsWindow;
    friend class audioConfig;

private:
    QString m_card;
    unsigned int m_bits;

    bool   m_subtunes;
    bool   m_bs2b;
    bool   m_replayGain;
    int    m_replayGainMode;

protected:
    settings() {}
    settings(const settings&);
    settings& operator= (const settings&);
    ~settings() {}

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
    int replayGainMode() const { return m_replayGainMode; }

    /// Bauer stereophonic-to-binaural DSP
    bool bs2b() const { return m_bs2b; }

    /// Default bittdepth
    unsigned int bits() const { return m_bits; }
};

#endif
