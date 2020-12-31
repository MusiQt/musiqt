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

#ifndef CONVERTERS_H
#define CONVERTERS_H

#include "converterBackend.h"
#include "quantizer.h"

template <typename I, typename O>
class resampler : public resamplerBackend
{
    quantizer<I, O>* _quantizer;

private:
    void init(const unsigned int fract);

public:
    resampler(const unsigned int srIn, const unsigned int srOut, const size_t size,
        const unsigned int channels, quantizer<I, O>* quantizer) :
        resamplerBackend(srIn, srOut, size, channels, sizeof(I), sizeof(O)),
        _quantizer(quantizer)
    {}
    ~resampler() { delete _quantizer; }

    /// Do the conversion
    size_t convert(const void* buf, size_t len) override;
};

/******************************************************************************/

template <typename I, typename O>
class converterDecimal : public converterBackend
{
    quantizer<I, O>* _quantizer;

private:
    void init(const unsigned int fract);

public:
    converterDecimal(const size_t size, const unsigned int channels, quantizer<I, O>* quantizer) :
        converterBackend(size, channels, sizeof(I), sizeof(O)),
        _quantizer(quantizer)
    {}
    ~converterDecimal() { delete _quantizer; }

    /// Do the conversion
    size_t convert(const void* buf, size_t len) override;
};

#endif
