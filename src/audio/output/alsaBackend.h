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

#ifndef ALSA_H
#define ALSA_H

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#ifdef HAVE_ALSA

#include "outputBackend.h"

#include <alsa/asoundlib.h>

#include <QStringList>

/*****************************************************************/

/**
 * ALSA output backend
*/
class alsaBackend : public outputBackend
{
private:
    snd_pcm_t *_pcmHandle;
    snd_mixer_t *_mixer;
    snd_mixer_elem_t *_mixerHandle;

    void *_buffer;

#if SND_LIB_VERSION >= 0x010014
    QStringList _devs;
#endif

private:
    alsaBackend();

    size_t setParams(unsigned int &sampleRate, const unsigned int channels, const unsigned int prec);

    bool initMixer(const char* dev);

public:
    virtual ~alsaBackend() {}

    static const char name[];

    /// Factory method
    static output* factory() { return new alsaBackend(); }

    /// Open
    size_t open(const unsigned int card, unsigned int &sampleRate,
                const unsigned int channels, const unsigned int prec) override;

    /// Close
    void close() override;

    /// Get buffer
    void *buffer() override { return _buffer; }

    /// Write data to output
    bool write(void* buffer, size_t bufferSize) override;

    /// Stop
    void stop() override;

    /// Set volume
    void volume(int vol) override;

    /// Get volume
    int volume() const override;
};

#endif

#endif
