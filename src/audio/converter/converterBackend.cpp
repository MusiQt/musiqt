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

#include "converterBackend.h"

#include <QDebug>

#include <cmath>
#include <cstdint>

resamplerBackend::resamplerBackend(unsigned int srIn, unsigned int srOut,
        unsigned int channels, unsigned int inputPrecision, unsigned int outputPrecision) :
    converter(channels, inputPrecision, outputPrecision),
    m_inputFrameSize(inputPrecision*m_channels),
    m_outputFrameSize(outputPrecision*m_channels)
{

    m_rate = static_cast<float>(srIn) / srOut;
    qDebug() << "Conversion ratio " << m_rate;
}

resamplerBackend::~resamplerBackend() = default;

size_t resamplerBackend::getBufferSize(size_t size)
{
    size_t const frames = size / m_outputFrameSize;
    size_t tmp = static_cast<size_t>(std::ceil(frames * m_rate));

    return tmp * m_inputFrameSize;
}

void resamplerBackend::increaseBufferSize(size_t size)
{
    m_outputSize = size;

    size_t bufferSize = getBufferSize(size);
    qDebug() << "resampler buffer size:" << bufferSize;
    m_buffer.resize(bufferSize);
}

size_t resamplerBackend::bufSize(size_t size)
{
    if (size > m_outputSize)
    {
        increaseBufferSize(size);
    }
    return getBufferSize(size) - m_dataPos;
}

/******************************************************************************/

converterBackend::converterBackend(unsigned int channels,
        unsigned int inputPrecision, unsigned int outputPrecision) :
    converter(channels, inputPrecision, outputPrecision)
{}

converterBackend::~converterBackend() = default;

size_t converterBackend::getBufferSize(size_t size)
{
    return size * m_frameRatio;
}

void converterBackend::increaseBufferSize(size_t size)
{
    m_outputSize = size;

    size_t bufferSize = getBufferSize(size);
    qDebug() << "converter buffer size:" << bufferSize;
    m_buffer.resize(bufferSize);
}

size_t converterBackend::bufSize(size_t size)
{
    if (size > m_outputSize)
    {
        increaseBufferSize(size);
    }
    return getBufferSize(size);
}
