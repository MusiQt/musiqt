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

#ifndef INPUT_PLUGIN_H
#define INPUT_PLUGIN_H

#include "input.h"

#include "utils.h"

#include <QIcon>
#include <QString>
#include <QSettings>

#define SECTION(key)	QString("%1 Settings/%2").arg(_name).arg(key)

#define gettext(x) x

/*****************************************************************/

class inputBackend : public input
{
private:
    QSettings settings;
    const char *_name;
    QIcon _icon;

    unsigned int _time;

protected:
    metaDataImpl _metaData;

private:
    inputBackend();
    inputBackend(const inputBackend&);
    inputBackend& operator=(const inputBackend&);

protected:
    inputBackend(const char name[], const unsigned char iconType[]=nullptr, unsigned int iconLen=0);

    /// Set song duration
    void time(unsigned int newTime) { _time=newTime; }

    /// Load int setting
    int load(const char* key, int defVal)
    {
        return settings.value(SECTION(key), defVal).toInt();
    }

    /// Load string setting
    QString load(const char* key, QString defVal)
    {
        return settings.value(SECTION(key), defVal).toString();
    }

    /// Save int setting
    void save(const char* key, int value)
    {
        settings.setValue(SECTION(key), value);
    }

    /// Save string setting
    void save(const char* key, QString value)
    {
        settings.setValue(SECTION(key), value);
    }

    /// Song is loaded
    void songLoaded(const QString& location);

    /// Rewind to start
    virtual bool rewind() { return false; }; // called only by seek(), will be removed

public:
    virtual ~inputBackend();

    /// Get backend name
    const char* name() const { return _name; }

    /// Get song info
    virtual const metaData* getMetaData() const override { return &_metaData; };

    /// Get song duration
    unsigned int time() const override { return _time; }

    /// Get fractional scale for fixed point types
    virtual unsigned int fract() const override { return 0; }

    /// Get filetype icon
    QIcon icon() const override { return _icon; }

    /// Seek specified position
    virtual bool seek(const int pos) override { return rewind(); }

    /// Song is loaded
    QString songLoaded() const override { return _metaData.getInfo(metaData::LOCATION); }

    /// Get number of subtunes
    virtual unsigned int subtunes() const override { return 0; }

    /// Get current subtune
    virtual unsigned int subtune() const override { return 0; }

    /// Change subtune
    virtual bool subtune(const unsigned int i) override { return false; }

    /// Gapless support
    virtual bool gapless() const override { return false; }

    /// Get Music directory
    virtual const QString getMusicDir(void) override { return QString::null; }

    virtual void loadSettings() {}

    virtual void saveSettings() {}
};

#endif
