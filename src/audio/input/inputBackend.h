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

#ifndef INPUT_PLUGIN_H
#define INPUT_PLUGIN_H

#include "input.h"

#include "utils.h"

#include <QIcon>
#include <QString>
#include <QSettings>

#define gettext(x) x

/*****************************************************************/

class inputBackend : public input, public inputConfig
{
private:
    QSettings settings;
    const char *m_name;
    QIcon m_icon;

    unsigned int m_time;

protected:
    metaDataImpl m_metaData;

private:
    inputBackend();
    inputBackend(const inputBackend&);
    inputBackend& operator=(const inputBackend&);

    inline QString section(const char* key) { return QString("%1 Settings/%2").arg(m_name).arg(key); }

protected:
    inputBackend(const char name[], const unsigned char iconType[]=nullptr, unsigned int iconLen=0);

    /// Set song duration
    void time(unsigned int newTime) { m_time = newTime; }

    /// Load int setting
    int load(const char* key, int defVal)
    {
        return settings.value(section(key), defVal).toInt();
    }

    /// Load string setting
    QString load(const char* key, QString defVal)
    {
        return settings.value(section(key), defVal).toString();
    }

    /// Save int setting
    void save(const char* key, int value)
    {
        settings.setValue(section(key), value);
    }

    /// Save string setting
    void save(const char* key, QString value)
    {
        settings.setValue(section(key), value);
    }

    /// Song is loaded
    void songLoaded(const QString& location);

public:
    virtual ~inputBackend();

    /// Get backend name
    const char* name() const { return m_name; }

    /// Get song info
    virtual const metaData* getMetaData() const override { return &m_metaData; }

    /// Get song duration in milliseconds
    unsigned int songDuration() const override { return m_time; }

    /// Get max play time in milliseconds, 0 if none
    virtual unsigned int maxPlayTime() const override { return 0; }

    /// Get fractional scale for fixed point types
    virtual unsigned int fract() const override { return 0; }

    /// Get filetype icon
    QIcon icon() const override { return m_icon; }

    /// Rewind to start
    virtual bool rewind() override { return seek(0); }

    /// Seek support
    virtual bool seekable() const override { return false; }

    /// Seek specified position
    virtual bool seek(int pos) override { return false; }

    /// Song is loaded
    QString songLoaded() const override { return m_metaData.getInfo(metaData::LOCATION); }

    /// Get number of subtunes
    virtual unsigned int subtunes() const override { return 0; }

    /// Get current subtune
    virtual unsigned int subtune() const override { return 0; }

    /// Change subtune
    virtual bool subtune(unsigned int i) override { return false; }

    /// Gapless support
    virtual bool gapless() const override { return false; }

    /// Get Music directory
    virtual const QString getMusicDir() const override { return QString(); }

    virtual void loadSettings() override {}

    virtual void saveSettings() override {}
};

#endif
