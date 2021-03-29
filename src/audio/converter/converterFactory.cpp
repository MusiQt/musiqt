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

converter* cFactory::get(audioFormat_t inFormat, audioFormat_t outFormat,
        size_t size, unsigned int fract)
{
    switch (inFormat.sampleType)
    {
    case sample_t::U8:
        return (inFormat.sampleRate != outFormat.sampleRate)
            ? new resampler<unsigned char, unsigned char>(inFormat.sampleRate, outFormat.sampleRate,
                size, outFormat.channels, new quantizerVoid<unsigned char>())
            : nullptr;
    case sample_t::S16:
        return (inFormat.sampleRate != outFormat.sampleRate)
            ? new resampler<short, short>(inFormat.sampleRate, outFormat.sampleRate,
                size, outFormat.channels, new quantizerVoid<short>())
            : nullptr;
    case sample_t::S24:
    case sample_t::S32:
        return nullptr;
    case sample_t::SAMPLE_FLOAT:
        if (inFormat.sampleRate != outFormat.sampleRate)
        {
            if (outFormat.sampleType == sample_t::U8)
            {
                qDebug() << "resampler float->U8";
                return (converter*)new resampler<float, unsigned char>(inFormat.sampleRate, outFormat.sampleRate,
                    size, outFormat.channels, new quantizerFloat<unsigned char>());
            }
            else if (outFormat.sampleType == sample_t::S16)
            {
                qDebug() << "resampler float->S16";
                return (converter*)new resampler<float, short>(inFormat.sampleRate, outFormat.sampleRate,
                    size, outFormat.channels, new quantizerFloat<short>());
            }
            else
            {
                qDebug() << "resampler float->float";
                //return (converter*)new resampler<float, float>(inFormat.sampleRate, outFormat.sampleRate,
                //    size, outFormat.channels, new quantizerVoid<float>());
                return nullptr; // FIXME
            }
        }
        else
        {
            if (outFormat.sampleType == sample_t::U8)
            {
                qDebug() << "converter float->U8";
                return (converter*)new converterDecimal<float, unsigned char>(size, outFormat.channels, new quantizerFloat<unsigned char>());
            }
            else if (outFormat.sampleType == sample_t::S16)
            {
                qDebug() << "converter float->S16";
                return (converter*)new converterDecimal<float, short>(size, outFormat.channels, new quantizerFloat<short>());
            }
            else
                return nullptr;
        }
    case sample_t::SAMPLE_FIXED:
        if (inFormat.sampleRate != outFormat.sampleRate)
        {
            if (outFormat.sampleType == sample_t::U8)
            {
                qDebug() << "resampler fixed->U8";
                return (converter*)new resampler<int, unsigned char>(inFormat.sampleRate, outFormat.sampleRate,
                    size, outFormat.channels, new quantizerFixed<unsigned char>(fract));
            }
            else
            {
                qDebug() << "resampler fixed->S16";
                return (converter*)new resampler<int, short>(inFormat.sampleRate, outFormat.sampleRate,
                    size, outFormat.channels, new quantizerFixed<short>(fract));
            }
        }
        else
        {
            if (outFormat.sampleType == sample_t::U8)
            {
                qDebug() << "converter fixed->U8";
                return (converter*)new converterDecimal<int, unsigned char>(size, outFormat.channels, new quantizerFixed<unsigned char>(fract));
            }
            else
            {
                qDebug() << "converter fixed->S16";
                return (converter*)new converterDecimal<int, short>(size, outFormat.channels, new quantizerFixed<short>(fract));
            }
        }
    default:
        return nullptr;
    }
}
