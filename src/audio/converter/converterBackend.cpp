/*
 *  Copyright (C) 2009-2017 Leandro Nini
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

#include "converterBackend.h"

#include <QDebug>

resamplerBackend::resamplerBackend(const unsigned int srIn, const unsigned int srOut, const size_t frames,
        const unsigned int channels, const unsigned int precision) :
    _channels(channels),
    _sampleSize(precision*channels),
    _data(0)
{
    qDebug() << "Conversion ratio " << (float)(srIn/srOut);
    qDebug() << "Sample size " << _sampleSize;

    _rate=(((unsigned int)srIn)<<16)/srOut;
    qDebug() << "_rate " << _rate;

    unsigned long tmp=((unsigned long)frames*(unsigned long)_rate);
    if (tmp&0xFFFFll)
        tmp+=0x10000ll;

    _bufsize=(tmp>>16)*_sampleSize;
    qDebug() << "input size: " << static_cast<int>(frames) << "; output size " << static_cast<int>(_bufsize/_sampleSize);
    _buffer=new char[_bufsize];
}

resamplerBackend::~resamplerBackend()
{
    delete [] _buffer;
}

/******************************************************************************/

converterBackend::converterBackend(const size_t frames, const unsigned int channels, const unsigned int precision) :
    _channels(channels),
    _sampleSize(precision*channels)
{
    qDebug() << "Sample size " << _sampleSize;

    _bufsize=frames*_sampleSize;
    qDebug() << "input size: " << static_cast<int>(frames) << "; output size " << static_cast<int>(_bufsize/_sampleSize);
    _buffer=new char[_bufsize];
}

converterBackend::~converterBackend()
{
    delete [] _buffer;
}
