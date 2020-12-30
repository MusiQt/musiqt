/*
 *  Copyright (C) 2009-2020 Leandro Nini
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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "converterFactory.h"

#include "converters.h"

cFactory* cFactory::instance()
{
    static cFactory o;
    return &o;
}

converter* cFactory::get(const unsigned int srIn, const unsigned int srOut,
    const size_t size, const unsigned int channels,
    const sample_t inputPrecision, const sample_t outputPrecision,
    const unsigned int fract)
{
    switch (inputPrecision)
    {
    case sample_t::U8:
        return (srIn != srOut) ? new resampler<unsigned char, unsigned char>(srIn, srOut, size, channels, new quantizerVoid<unsigned char>()) : nullptr;
    case sample_t::S16:
        return (srIn != srOut) ? new resampler<short, short>(srIn, srOut, size, channels, new quantizerVoid<short>()) : nullptr;
    case sample_t::S24:
    case sample_t::S32:
        return nullptr;
    case sample_t::SAMPLE_FLOAT:
        if (srIn != srOut)
        {
            if (outputPrecision == sample_t::U8)
            {
                qDebug() << "resampler float->U8";
                return (converter*)new resampler<float, unsigned char>(srIn, srOut, size, channels, new quantizerFloat<unsigned char>());
            }
            else
            {
                qDebug() << "resampler float->S16";
                return (converter*)new resampler<float, short>(srIn, srOut, size, channels, new quantizerFloat<short>());
            }
        }
        else
        {
            if (outputPrecision == sample_t::U8)
            {
                qDebug() << "converter float->U8";
                return (converter*)new converterDecimal<float, unsigned char>(size, channels, new quantizerFloat<unsigned char>());
            }
            else
            {
                qDebug() << "converter float->S16";
                return (converter*)new converterDecimal<float, short>(size, channels, new quantizerFloat<short>());
            }
        }
    case sample_t::SAMPLE_FIXED:
        if (srIn != srOut)
        {
            if (outputPrecision == sample_t::U8)
            {
                qDebug() << "resampler fixed->U8";
                return (converter*)new resampler<int, unsigned char>(srIn, srOut, size, channels, new quantizerFixed<unsigned char>(fract));
            }
            else
            {
                qDebug() << "resampler fixed->S16";
                return (converter*)new resampler<int, short>(srIn, srOut, size, channels, new quantizerFixed<short>(fract));
            }
        }
        else
        {
            if (outputPrecision == sample_t::U8)
            {
                qDebug() << "converter fixed->U8";
                return (converter*)new converterDecimal<int, unsigned char>(size, channels, new quantizerFixed<unsigned char>(fract));
            }
            else
            {
                qDebug() << "converter fixed->S16";
                return (converter*)new converterDecimal<int, short>(size, channels, new quantizerFixed<short>(fract));
            }
        }
    default:
        return nullptr;
    }
}
