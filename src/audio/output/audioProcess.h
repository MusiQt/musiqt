/*
 *  Copyright (C) 2020-2021 Leandro Nini
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

#ifndef AUDIOPROCESS_H
#define AUDIOPROCESS_H

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#ifdef HAVE_BS2B
#  include <bs2b.h>
#endif

#include <QtEndian>
#include <QDebug>

class audioProcess
{
protected:
#ifdef HAVE_BS2B
    t_bs2bdp m_bs2bdp;
#endif

    void initBs2b(int sampleRate)
    {
#ifdef HAVE_BS2B
        m_bs2bdp = bs2b_open();
        if (m_bs2bdp)
        {
            qDebug() << "bs2b enabled";
            bs2b_set_srate(m_bs2bdp, sampleRate);
            bs2b_set_level(m_bs2bdp, BS2B_DEFAULT_CLEVEL);
        }
#endif
    }

public:
    audioProcess()
#ifdef HAVE_BS2B
        : m_bs2bdp(0)
#endif
    {}

    virtual ~audioProcess() {}

    virtual void init(int sampleRate) =0;

    void finish()
    {
#ifdef HAVE_BS2B
        if (m_bs2bdp)
        {
            bs2b_close(m_bs2bdp);
            m_bs2bdp = 0;
        }
#endif
    }

    virtual void process(void* buffer, size_t size) =0;
};

/*****************************************************************/

class audioProcess8 : public audioProcess
{
    void init(int sampleRate)
    {
        initBs2b(sampleRate);
    }

    void process(void* buffer, size_t size)
    {
#ifdef HAVE_BS2B
        if (m_bs2bdp)
        {
            uint8_t *buf = (uint8_t*)buffer;
            bs2b_cross_feed_u8(m_bs2bdp, buf, size/2);
        }
#endif
    }
};

/*****************************************************************/

class audioProcess16 : public audioProcess
{
    void init(int sampleRate)
    {
        initBs2b(sampleRate);
    }

    void process(void* buffer, size_t size)
    {
#if (Q_BYTE_ORDER == Q_BIG_ENDIAN)
        {
            //Swap bytes on big endian machines
            quint16 *buf = (quint16*)buffer;
            const quint16 *end = buf+(size/2);
            do {
                const quint16 tmp = *buf;
                *buf++ = qToLittleEndian(tmp);
            } while (buf<end);
        }
#endif

#ifdef HAVE_BS2B
        if (m_bs2bdp)
        {
            int16_t *buf = (int16_t*)buffer;
            bs2b_cross_feed_s16le(m_bs2bdp, buf, size/4);
        }
#endif
    }
};

/*****************************************************************/

class audioProcess32 : public audioProcess
{
    void init(int sampleRate)
    {
        initBs2b(sampleRate);
    }

    void process(void* buffer, size_t size)
    {
#if (Q_BYTE_ORDER == Q_BIG_ENDIAN)
        {
            //Swap bytes on big endian machines
            quint32 *buf = (quint32*)buffer;
            const quint32 *end = buf+(size/4);
            do {
                const quint32 tmp = *buf;
                *buf++ = qToLittleEndian(tmp);
            } while (buf<end);
        }
#endif

#ifdef HAVE_BS2B
        if (m_bs2bdp)
        {
            int32_t *buf = (int32_t*)buffer;
            bs2b_cross_feed_s32le(m_bs2bdp, buf, size/4);
        }
#endif
    }
};

/*****************************************************************/

class audioProcessFloat : public audioProcess
{
    void init(int sampleRate)
    {
        initBs2b(sampleRate);
    }

    void process(void* buffer, size_t size)
    {
#ifdef HAVE_BS2B
        if (m_bs2bdp)
        {
            float *buf = (float*)buffer;
            bs2b_cross_feed_fle(m_bs2bdp, buf, size/8);
        }
#endif
    }
};

#endif
