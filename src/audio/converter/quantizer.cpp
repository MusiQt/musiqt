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

#include "quantizer.h"

#include <limits>

// LCG
unsigned int random(unsigned int val)
{
    return val * 1664525 + 1013904223;
}

template<typename O>
quantizerFixed<O>::quantizerFixed(const unsigned int fract)
{
    _random[0][0]=3686734;
    _random[0][1]=86526882;
    _random[1][0]=268268;
    _random[1][1]=7628932;

    qDebug() << "quantizerFixed " << static_cast<int>(sizeof(O)) << " bytes";
    _scalebits=fract+1-(sizeof(O)<<3);
    _mask=(1L<<_scalebits)-1;
    _clip=(1L<<fract);
}

template quantizerFixed<unsigned char>::quantizerFixed(const unsigned int fract);
template quantizerFixed<short>::quantizerFixed(const unsigned int fract);

template<typename O>
inline int quantizerFixed<O>::get32(const int sample, const unsigned int channel)
{
    const unsigned int r1=random(_random[channel][0])&_mask;
    const unsigned int r2=random(_random[channel][1])&_mask;

    // Dither
    int output=sample+
        r1+r2+
        (1L<<(_scalebits-1));

    // Clip
    if (output>_clip-1)
        output=_clip-1;
    else if (output<-_clip)
        output=-_clip;

    return output;
}

template<>
unsigned char quantizerFixed<unsigned char>::get(const int sample, const unsigned int channel)
{
    return (unsigned char)(get32(sample, channel)>>_scalebits)+128;
}

template<>
short quantizerFixed<short>::get(const int sample, const unsigned int channel)
{
    return (short)(get32(sample, channel)>>_scalebits);
}

/******************************************************************************/

template<typename O>
quantizerFloat<O>::quantizerFloat()
{
    _random[0][0]=3686734;
    _random[0][1]=86526882;
    _random[1][0]=268268;
    _random[1][1]=7628932;

    qDebug() << "quantizerFloat " << static_cast<int>(sizeof(O)) << " bytes";
}

template quantizerFloat<unsigned char>::quantizerFloat();
template quantizerFloat<short>::quantizerFloat();

template<typename O>
inline int quantizerFloat<O>::get32(const float sample, const unsigned int channel, const int max)
{
    const float r1=(float)random(_random[channel][0])/(float)std::numeric_limits<unsigned int>::max();
    const float r2=(float)random(_random[channel][1])/(float)std::numeric_limits<unsigned int>::max();

    // Dither
    int output=(int)(sample*(float)max+
        r1+r2+
        0.5f);

    // Clip
    if (output>max-1)
        output=max-1;
    else if (output<-max)
        output=-max;

    return output;
}

template<>
unsigned char quantizerFloat<unsigned char>::get(const float sample, const unsigned int channel)
{
    return (unsigned char)(get32(sample, channel, 128))+128;
}

template<>
short quantizerFloat<short>::get(const float sample, const unsigned int channel)
{
    return (short)(get32(sample, channel, 32768));
}
