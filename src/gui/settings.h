/*
 *  Copyright (C) 2008-2017 Leandro Nini
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
#include <QVBoxLayout>
#include <QLabel>

class inputConfig;

class settingsWindow : public QDialog
{
    Q_OBJECT

private:
    QVBoxLayout	*_replayGainBox;
    QLabel *colorLabel;

private:
    settingsWindow() {}
    settingsWindow(const settingsWindow&);
    settingsWindow& operator=(const settingsWindow&);

private slots:
    void setSubtunes(int val);
    void setAutobk(int val);
#ifdef HAVE_BS2B // FIXME moc ignores this
    void setBs2b(int val);
#endif
    void setReplaygain(bool val);
    void setReplaygainMode(int val);

public:
    settingsWindow(QWidget* win, inputConfig* i);
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
    QString _apiName;
    QString _card;
    unsigned int _bits;

    bool   _subtunes;
    bool   _autoBk;
#ifdef HAVE_BS2B
    bool   _bs2b;
#endif
    bool   _replayGain;
    int    _replayGainMode;

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

    /// Audio API
    const QString& apiName() const { return _apiName; }

    /// Soundcard
    const QString& card() const { return _card; }

    /// Play subtunes
    bool subtunes() const { return _subtunes; }

    /// Automatic backend selection
    bool autoBk() const { return _autoBk; }

    /// Replay Gain
    bool replayGain() const { return _replayGain; }

    /// Replay Gain Mode
    int replayGainMode() const { return _replayGainMode; }

#ifdef HAVE_BS2B
    /// Bauer stereophonic-to-binaural DSP
    bool bs2b() const { return _bs2b; }
#endif

    /// Default bittdepth
    unsigned int bits() const { return _bits; }
};

#endif
