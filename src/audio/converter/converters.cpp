/*
 *  Copyright (C) 2009-2021 Leandro Nini
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
size_t resampler<I, O>::convert(const void* buf, size_t len)
{
    I* const in = (I*)m_buffer.data();
    O* const out = (O*)buf;

    size_t idx = 0;
    unsigned int error = 0;
    const size_t samples = len/sizeof(I);
    for (size_t j=0; j<samples; j+=channels)
    {
        for (unsigned int c=0; c<channels; c++)
        {
            const I val = in[idx+c];
            out[j+c] = _quantizer->get(val+(I)(((unsigned int)(in[idx+c+channels]-val)*error)>>16), c);
        }
        error += m_rate;
        while (error >= 0x10000)
        {
            error -= 0x10000;
            idx += channels;
        }
    }

    const size_t l = m_buffer.size()/sizeof(I);
    qDebug() << "resamplerDecimal idx: " << static_cast<int>(idx) << "; l: " << static_cast<int>(l);

    m_dataPos = (l < idx) ? (l-idx)*sizeof(I) : 0;
    for (size_t j=idx; j<l; j+=channels)
    {
        for (unsigned int c=0; c<channels; c++)
            in[c] = in[j+c];
    }

    return samples * sizeof(O);
}

template size_t resampler<unsigned char, unsigned char>::convert(const void* buf, const size_t len);
template size_t resampler<short, short>::convert(const void* buf, const size_t len);
template size_t resampler<int, unsigned char>::convert(const void* buf, const size_t len);
template size_t resampler<int, short>::convert(const void* buf, const size_t len);
template size_t resampler<float, unsigned char>::convert(const void* buf, const size_t len);
template size_t resampler<float, short>::convert(const void* buf, const size_t len);
template size_t resampler<float, float>::convert(const void* buf, const size_t len);

/******************************************************************************/

template <typename I, typename O>
size_t converterDecimal<I, O>::convert(const void* buf, size_t len)
{
    I* const in = (I*)m_buffer.data();
    O* const out = (O*)buf;

    const size_t samples = len/sizeof(I);
    for (size_t j=0; j<samples; j+=channels)
    {
        for (unsigned int c=0; c<channels; c++)
        {
            out[j+c] = _quantizer->get(in[j+c], c);
        }
    }

    return samples * sizeof(O);
}

template size_t converterDecimal<int, unsigned char>::convert(const void* buf, const size_t len);
template size_t converterDecimal<int, short>::convert(const void* buf, const size_t len);
template size_t converterDecimal<float, unsigned char>::convert(const void* buf, const size_t len);
template size_t converterDecimal<float, short>::convert(const void* buf, const size_t len);
