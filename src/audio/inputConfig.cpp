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

#include "inputConfig.h"

inputConfig::inputConfig(const char name[], const unsigned char iconType[], unsigned int iconLen) :
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

int inputConfig::load(const char* key, int defVal)
{
    return m_settings.value(section(key), defVal).toInt();
}

QString inputConfig::load(const char* key, QString defVal)
{
    return m_settings.value(section(key), defVal).toString();
}

void inputConfig::save(const char* key, int value)
{
    m_settings.setValue(section(key), value);
}

void inputConfig::save(const char* key, QString value)
{
    m_settings.setValue(section(key), value);
}
