/*
 *  Copyright (C) 2009-2023 Leandro Nini
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

resamplerBackend::resamplerBackend(unsigned int srIn, unsigned int srOut,
        unsigned int channels, unsigned int inputPrecision, unsigned int outputPrecision) :
    converter(channels, inputPrecision, outputPrecision),
    m_dataPos(0),
    m_inputFrameSize(inputPrecision*m_channels),
    m_outputFrameSize(outputPrecision*m_channels)
{
    qDebug() << "Conversion ratio " << (float)(srIn/srOut);

    m_rate = (((unsigned int)srIn)<<16)/srOut;
    qDebug() << "m_rate " << m_rate;
}

resamplerBackend::~resamplerBackend() {}

void resamplerBackend::setBufferSize(size_t size)
{
    m_inputSize = size;

    size_t const frames = size / m_outputFrameSize;
    unsigned long tmp = ((unsigned long)frames * (unsigned long)m_rate);
    if (tmp & 0xFFFFll)
        tmp += 0x10000ll;

    size_t const bufferSize = (tmp>>16) * m_inputFrameSize;
    qDebug() << "resampler buffer size:" << bufferSize;
    m_buffer.resize(bufferSize);
}

size_t resamplerBackend::bufSize(size_t size)
{
    if (size > m_inputSize) {
        setBufferSize(size);
    }
    return (size*m_frameRatio)-m_dataPos;
}

/******************************************************************************/

converterBackend::converterBackend(unsigned int channels,
        unsigned int inputPrecision, unsigned int outputPrecision) :
    converter(channels, inputPrecision, outputPrecision)
{}

converterBackend::~converterBackend() {}

void converterBackend::setBufferSize(size_t size)
{
    m_inputSize = size;

    size_t const bufferSize = size * m_frameRatio;
    qDebug() << "converter buffer size:" << bufferSize;
    m_buffer.resize(bufferSize);
}

size_t converterBackend::bufSize(size_t size)
{
    if (size > m_inputSize) {
        setBufferSize(size);
    }
    return size*m_frameRatio;
}
