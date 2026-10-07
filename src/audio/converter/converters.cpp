/*
 *  Copyright (C) 2009-2026 Leandro Nini
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

#include <cstring>

template <typename I, typename O>
size_t resampler<I, O>::convert(const void* buf, size_t len, size_t ilen)
{
    I* const in = (I*)m_buffer.data();
    O* const out = (O*)buf;

    const size_t samples = len/sizeof(O);
    size_t idx_i = 0;
    size_t idx_o = 0;
    while (idx_o < samples)
    {
        for (unsigned int c=0; c<m_channels; c++)
        {
            const float a = (float)in[idx_i+c];
            const float b = (float)in[idx_i+m_channels+c];
            const I lerp = (I)(a + ((b-a)*error));
            out[idx_o+c] = _quantizer->get(lerp, c);
        }
        idx_o += m_channels;
        error += m_rate;
        while (error >= 1.f)
        {
            error -= 1.f;
            idx_i += m_channels;
        }
    }

    const size_t l = ilen/sizeof(I);
    qDebug().nospace() << "resamplerDecimal idx_i: " << static_cast<int>(idx_i) << ", l: " << static_cast<int>(l);

    if (idx_i < l)
    {
        m_dataPos = (l-idx_i)*sizeof(I);
        std::memmove(in, in+idx_i, m_dataPos*m_channels);
    }
    else
        m_dataPos = 0;

    return idx_o * sizeof(O);
}

template size_t resampler<unsigned char, unsigned char>::convert(const void* buf, const size_t len, size_t ilen);
template size_t resampler<short, short>::convert(const void* buf, const size_t len, size_t ilen);
template size_t resampler<int, unsigned char>::convert(const void* buf, const size_t len, size_t ilen);
template size_t resampler<int, short>::convert(const void* buf, const size_t len, size_t ilen);
template size_t resampler<float, unsigned char>::convert(const void* buf, const size_t len, size_t ilen);
template size_t resampler<float, short>::convert(const void* buf, const size_t len, size_t ilen);
template size_t resampler<float, float>::convert(const void* buf, const size_t len, size_t ilen);

/******************************************************************************/

template <typename I, typename O>
size_t converterDecimal<I, O>::convert(const void* buf, size_t, size_t ilen)
{
    I* const in = (I*)m_buffer.data();
    O* const out = (O*)buf;

    const size_t samples = ilen/sizeof(I);
    for (size_t j=0; j<samples; j+=m_channels)
    {
        for (unsigned int c=0; c<m_channels; c++)
        {
            out[j+c] = _quantizer->get(in[j+c], c);
        }
    }

    return samples * sizeof(O);
}

template size_t converterDecimal<int, unsigned char>::convert(const void* buf, const size_t len, size_t ilen);
template size_t converterDecimal<int, short>::convert(const void* buf, const size_t len, size_t ilen);
template size_t converterDecimal<float, unsigned char>::convert(const void* buf, const size_t len, size_t ilen);
template size_t converterDecimal<float, short>::convert(const void* buf, const size_t len, size_t ilen);
