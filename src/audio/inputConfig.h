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

#ifndef INPUTCONFIG_H
#define INPUTCONFIG_H

#include "iconFactory.h"

#include <QIcon>
#include <QSettings>
#include <QString>

class QWidget;

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

#endif
