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

#include "inputBackend.h"

extern const unsigned char iconDefault[76] =
{
    0x47,0x49,0x46,0x38,0x39,0x61,0x10,0x00,0x10,0x00,0x80,0x01,0x00,0x00,0x00,0x00,
    0xff,0xff,0xff,0x21,0xf9,0x04,0x01,0x0a,0x00,0x01,0x00,0x2c,0x00,0x00,0x00,0x00,
    0x10,0x00,0x10,0x00,0x00,0x02,0x23,0x8c,0x8f,0xa9,0xcb,0x01,0xad,0x1e,0x92,0x8b,
    0x1a,0x9b,0x1e,0xd5,0xd9,0xf9,0xfb,0x79,0xdc,0x28,0x96,0x24,0x09,0x72,0x69,0x15,
    0xaa,0xdd,0xd4,0x60,0x18,0x0c,0xd5,0x36,0x53,0x00,0x00,0x3b
};

inputBackend::inputBackend(const char name[], const unsigned char* iconType, unsigned int iconLen) :
    _name(name),
    _time(0)
{
    // Use default icon if not provided
    if (iconType == nullptr)
    {
        iconType = iconDefault;
        iconLen = 76;
    }

    QPixmap pixmap;
    if (pixmap.loadFromData(iconType, iconLen))
        _icon = QIcon(pixmap);
}

inputBackend::~inputBackend() {}

void inputBackend::songLoaded(const QString& location)
{
    if (!location.isEmpty())
        _metaData.addInfo(metaData::LOCATION, location);
    else
        _metaData.clearInfo();
}
