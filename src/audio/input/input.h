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

#ifndef INPUT_H
#define INPUT_H

#include "inputTypes.h"
#include "metaData.h"

#include "iconFactory.h"

#include <QIcon>
#include <QSettings>
#include <QString>

class QWidget;

#define gettext(x) x

/**
 * base class for input config
 */
class inputConfig
{
private:
    const char *m_name;

    QIcon m_icon;

    QSettings m_settings;

private:
    inline QString section(const char* key) { return QString("%1 Settings/%2").arg(name()).arg(key); }

protected:
    const char* name() const { return m_name; }

protected:
    inputConfig(const char name[], const unsigned char iconType[]=nullptr, unsigned int iconLen=0);

    /// Load int setting
    int load(const char* key, int defVal);

    /// Load string setting
    QString load(const char* key, QString defVal);

    /// Save int setting
    void save(const char* key, int value);

    /// Save string setting
    void save(const char* key, QString value);

public:
    virtual ~inputConfig() {}

    /// Open config dialog
    virtual QWidget* config(QWidget* parent=nullptr) =0;

    /// Get filetype icon
    QIcon icon() const { return m_icon; }

    /// Get Music directory
    virtual const QString getMusicDir() const { return QString(); }

    virtual void loadSettings() {}

    virtual void saveSettings() {}
};

/**
 * base class for input backends
 */
class input
{
public:
    class loadError {};

private:
    unsigned int m_time;

protected:
    metaDataImpl m_metaData;

protected:
    input() : m_time(0) {}

    /// Song is loaded
    void songLoaded(const QString& location);

    /// Set song duration
    void setDuration(unsigned int newTime) { m_time = newTime; }

public:
    virtual ~input() {}

    /// Get song info
    const metaData* getMetaData() const { return &m_metaData; }

    /// Get song duration in milliseconds
    unsigned int songDuration() const { return m_time; }

    /// Get max play time in milliseconds, 0 if none
    virtual unsigned int maxPlayTime() const { return 0; }

    /// Get samplerate
    virtual unsigned int samplerate() const =0;

    /// Get channels
    virtual unsigned int channels() const =0;

    /// Get precision
    virtual sample_t precision() const =0;

    /// Get fractional scale for fixed point types
    virtual unsigned int fract() const { return 0; }

    /// Rewind to start
    virtual bool rewind() { return seek(0); }

    /// Seek support
    virtual bool seekable() const { return false; }

    /// Seek specified position
    virtual bool seek(int pos) { return false; }

    /// Callback function
    virtual size_t fillBuffer(void* buffer, size_t bufferSize) =0;

    /// Song is loaded
    QString songLoaded() const { return m_metaData.getInfo(metaData::LOCATION); }

    /// Get number of subtunes
    virtual unsigned int subtunes() const { return 0; }

    /// Get current subtune
    virtual unsigned int subtune() const { return 0; }

    /// Change subtune
    virtual bool subtune(unsigned int i) { return false; }

    /// Gapless support
    virtual bool gapless() const { return false; }
};

#endif
