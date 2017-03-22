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

#ifndef OUTPUT_PLUGIN_H
#define OUTPUT_PLUGIN_H

#include "output.h"

#include <new>
#include <QList>
#include <QSettings>

/*****************************************************************/

#define SECTION(key) QString("Audio Settings/%1").arg(key)

class outputBackend : public output
{
private:
    QSettings settings;
    const char *_name;

    QList<QString> _devices;

private:
    outputBackend();
    outputBackend(const outputBackend&);
    outputBackend& operator=(const outputBackend&);

protected:
    outputBackend(const char name[]) : _name(name) {}

    /// Add a device to the list
    void addDevice(const char* dev) { _devices.append(dev); }

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

public:
    virtual ~outputBackend() {}

    /// Get backend name
    const char* name() const override { return _name; }

    /// Get number of available devices
    unsigned int devices() const override { return _devices.size(); }

    /// Get device name
    const QString device(const unsigned int i) const override { return _devices[i]; }

    /// Pause
    virtual void pause() override {}

    /// Stop
    virtual void stop() override {}

    /// Set volume
    virtual void volume(int vol) override {}

    /// Get volume
    virtual int volume() const override { return -1; }
};

#endif
