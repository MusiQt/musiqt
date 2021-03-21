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
#include <QWidget>

/**
 * base class for input config
 */
class inputConfig
{
private:
    const char *m_name;

    QSettings m_settings;

    QIcon m_icon;

private:
    inline QString section(const char* key) { return QString("%1 Settings/%2").arg(name()).arg(key); }

protected:
    const char* name() const { return m_name; }

protected:
    inputConfig(const char name[], const unsigned char iconType[]=nullptr, unsigned int iconLen=0) :
        m_name(name)
    {
        // Use default icon if not provided
        if (iconType == nullptr)
        {
            m_icon = GET_ICON(icon_backend);
        }
        else
        {
            QPixmap pixmap;
            if (pixmap.loadFromData(iconType, iconLen))
                m_icon = QIcon(pixmap);
        }
    }

    /// Load int setting
    int load(const char* key, int defVal)
    {
        return m_settings.value(section(key), defVal).toInt();
    }

    /// Load string setting
    QString load(const char* key, QString defVal)
    {
        return m_settings.value(section(key), defVal).toString();
    }

    /// Save int setting
    void save(const char* key, int value)
    {
        m_settings.setValue(section(key), value);
    }

    /// Save string setting
    void save(const char* key, QString value)
    {
        m_settings.setValue(section(key), value);
    }

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
 * interface class for input backends
 */
class input
{
protected:
    /// Close file
    virtual void close() =0;

public:
    virtual ~input() {}

    /// Get song info
    virtual const metaData* getMetaData() const =0;

    /// Get song duration in milliseconds
    virtual unsigned int songDuration() const =0;

    /// Get max play time in milliseconds, 0 if none
    virtual unsigned int maxPlayTime() const =0;

    /// Get samplerate
    virtual unsigned int samplerate() const =0;

    /// Get channels
    virtual unsigned int channels() const =0;

    /// Get precision
    virtual sample_t precision() const =0;

    /// Get fractional scale for fixed point types
    virtual unsigned int fract() const =0;

    /// Open file
    virtual bool open(const QString& fileName) =0;

    /// Rewind to start
    virtual bool rewind() =0;

    /// Seek support
    virtual bool seekable() const =0;

    /// Seek specified position
    virtual bool seek(int pos) =0;

    /// Callback function
    virtual size_t fillBuffer(void* buffer, size_t bufferSize) =0;

    /// Song is loaded
    virtual QString songLoaded() const =0;

    /// Get number of subtunes
    virtual unsigned int subtunes() const =0;

    /// Get current subtune
    virtual unsigned int subtune() const =0;

    /// Change subtune
    virtual bool subtune(unsigned int i) =0;

    /// Gapless support
    virtual bool gapless() const =0;
};

#endif
