/*
 *  Copyright (C) 2011-2017 Leandro Nini
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

#ifndef IMAGEDATA_H
#define IMAGEDATA_H

#include <QString>

class imageData
{
private:
    size_t _size;
    char* _data;
    QString _type;

public:
    imageData(const size_t size, const char *data, QString &type) :
        _size(size),
        _data(new char[size]),
        _type(type)
    {
        memcpy(_data, data, size);
    }

    ~imageData() { delete [] _data; }

    size_t size(void) const { return _size; }
    char* data(void) const { return _data; }
    const QString& type(void) const { return _type; }
};

#endif
