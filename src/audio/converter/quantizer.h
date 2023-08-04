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

#ifndef QUANTIZER_H
#define QUANTIZER_H

#include <QDebug>

template <typename I, typename O>
class quantizer
{
public:
    virtual ~quantizer() {}

    /// Get dithered sample
    virtual O get(const I sample, const unsigned int channel) =0;
};

/******************************************************************************/

template <typename T>
class quantizerVoid final : public quantizer<T, T>
{
private:
    quantizerVoid(const quantizerVoid&);
    quantizerVoid& operator=(const quantizerVoid&);
public:
    quantizerVoid() { qDebug() << "quantizerVoid" <<  static_cast<int>(sizeof(T)) << "bytes"; };
    virtual ~quantizerVoid() {}

    /// Get dithered sample
    inline T get(const T sample, [[maybe_unused]] const unsigned int channel) override { return sample; }
};

/******************************************************************************/

template<typename O>
class quantizerFixed final : public quantizer<int, O>
{
protected:
    unsigned int _random[2][2];

    int _scalebits;
    int _clip;
    int _mask;

private:
    quantizerFixed(const quantizerFixed&);
    quantizerFixed& operator=(const quantizerFixed&);

    int get32(const int sample, const unsigned int channel);

public:
    quantizerFixed(const unsigned int fract);
    virtual ~quantizerFixed() {}

    /// Get dithered sample
    O get(const int sample, const unsigned int channel) override;
};

/******************************************************************************/

template<typename O>
class quantizerFloat final : public quantizer<float, O>
{
protected:
    unsigned int _random[2][2];

private:
    quantizerFloat(const quantizerFloat<O>&);
    quantizerFloat<O>& operator=(const quantizerFloat<O>&);

    int get32(const float sample, const unsigned int channel, const int max);

public:
    quantizerFloat();
    virtual ~quantizerFloat() {}

    /// Get dithered sample
    O get(const float sample, const unsigned int channel) override;
};

#endif
