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

#include "converters.h"

template <typename I, typename O>
size_t resampler<I, O>::convert(const void* buf, const size_t len)
{
    I* const in = (I*)_buffer;
    O* const out = (O*)buf;

    size_t idx = 0;
    unsigned int error = 0;
    const size_t m = len/sizeof(O);
    for (size_t j=0; j<m; j+=_channels)
    {
        for (unsigned int c=0; c<_channels; c++)
        {
            const I val = in[idx+c];
            out[j+c] = _quantizer->get(val+(I)(((unsigned int)(in[idx+c+_channels]-val)*error)>>16), c);
        }
        error += _rate;
        while (error >= 0x10000)
        {
            error -= 0x10000;
            idx += _channels;
        }
    }

    const size_t l = _bufsize/sizeof(I);
    qDebug() << "resamplerDecimal idx: " << static_cast<int>(idx) << "; l: " << static_cast<int>(l);

    _data = (l < idx) ? (l-idx)*sizeof(I) : 0;
    for (size_t j=idx; j<l; j+=_channels)
    {
        for (unsigned int c=0; c<_channels; c++)
            in[c] = in[j+c];
    }

    return len;
}

template size_t resampler<unsigned char, unsigned char>::convert(const void* buf, const size_t len);
template size_t resampler<short, short>::convert(const void* buf, const size_t len);
template size_t resampler<int, unsigned char>::convert(const void* buf, const size_t len);
template size_t resampler<int, short>::convert(const void* buf, const size_t len);
template size_t resampler<float, unsigned char>::convert(const void* buf, const size_t len);
template size_t resampler<float, short>::convert(const void* buf, const size_t len);

/******************************************************************************/

template <typename I, typename O>
size_t converterDecimal<I, O>::convert(const void* buf, const size_t len)
{
    I* const in = (I*)_buffer;
    O* const out = (O*)buf;

    const size_t m = len/sizeof(O);
    for (size_t j=0; j<m; j+=_channels)
    {
        for (unsigned int c=0; c<_channels; c++)
        {
            out[j+c] = _quantizer->get(in[j+c], c);
        }
    }

    return len;
}

template size_t converterDecimal<int, unsigned char>::convert(const void* buf, const size_t len);
template size_t converterDecimal<int, short>::convert(const void* buf, const size_t len);
template size_t converterDecimal<float, unsigned char>::convert(const void* buf, const size_t len);
template size_t converterDecimal<float, short>::convert(const void* buf, const size_t len);
