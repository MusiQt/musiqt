/*
 *  Copyright (C) 2009-2023 Leandro Nini
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

#ifndef CONVERTER_H
#define CONVERTER_H

#include <cstdlib>

#include <QByteArray>
#include <QDebug>

class converter
{
protected:
    const unsigned int channels;
    const unsigned int frameRatio;

    QByteArray m_buffer;

protected:
    converter(unsigned int channels, unsigned int inputPrecision, unsigned int outputPrecision) :
        channels(channels),
        frameRatio(inputPrecision/outputPrecision)
    {
        qDebug() << "Frame ratio " << frameRatio;
    }

public:
    virtual ~converter() {}

    /// Get pointer to buffer
    virtual char* buffer() =0;

    /// Get buffer size
    virtual size_t bufSize(size_t size) const =0;

    /// Do the conversion
    virtual size_t convert(const void* out, size_t len) =0;
};

#endif
