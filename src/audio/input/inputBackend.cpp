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

#include "inputBackend.h"

#include "iconFactory.h"

inputBackend::inputBackend(const char name[], const unsigned char* iconType, unsigned int iconLen) :
    _name(name),
    _time(0)
{
    // Use default icon if not provided
    if (iconType == nullptr)
    {
        _icon = GET_ICON(icon_backend);
    }
    else
    {
        QPixmap pixmap;
        if (pixmap.loadFromData(iconType, iconLen))
            _icon = QIcon(pixmap);
    }
}

inputBackend::~inputBackend() {}

void inputBackend::songLoaded(const QString& location)
{
    if (!location.isEmpty())
        _metaData.addInfo(metaData::LOCATION, location);
    else
        _metaData.clearInfo();
}
