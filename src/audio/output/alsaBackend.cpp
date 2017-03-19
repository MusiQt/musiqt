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

#include "alsaBackend.h"

#ifdef HAVE_ALSA

#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include <QDebug>

const char alsaBackend::name[]="ALSA";

/*****************************************************************/

alsaBackend::alsaBackend() :
    outputBackend(name),
    _pcmHandle(nullptr),
    _mixer(nullptr),
    _buffer(nullptr)
{
    // Check devices
#if SND_LIB_VERSION >= 0x010014
    void **hints;

    const int err=snd_device_name_hint(-1, "pcm", &hints);
    if (err<0)
    {
        qWarning() << "Error: " << snd_strerror(err);
        return;
    }

    void **h=hints;
    while (*h)
    {
        char *name=snd_device_name_get_hint(*h, "NAME");
        qDebug() << name;
        //char *type=snd_device_name_get_hint(*h, "IOID"); //"Input"/"Output"
        //qDebug() << type;
        if (QString(name).startsWith("default"))
        {
            _devs.append(name);

            char *desc=snd_device_name_get_hint(*h, "DESC");
            qDebug() << desc;
            QString temp(desc);
            addDevice(temp.left(temp.indexOf(',')).toStdString().c_str());
            free(desc);
        }

        free(name);
        h++;
    }
    snd_device_name_free_hint(hints);
#else
    int card=-1;
    snd_card_next(&card);
    char* devName;
    while (card>=0)
    {
        snd_card_get_name(card, &devName);
        addDevice(devName);
        snd_card_next(&card);
    }
#endif
}

size_t alsaBackend::open(const unsigned int card, unsigned int &sampleRate,
                         const unsigned int channels, const unsigned int prec)
{
#if SND_LIB_VERSION >= 0x010014
    QString devName=_devs[card];
#else
    QString devName=QString("hw:%1").arg(card);
#endif
    qDebug() << "Opening device: " << devName;
    const int err=snd_pcm_open(&_pcmHandle, devName.toStdString().c_str(), SND_PCM_STREAM_PLAYBACK, 0);
    if (err<0)
    {
        qWarning() << "Error: " << snd_strerror(err);
        return 0;
    }

    const size_t bufSize=setParams(sampleRate, channels, prec);
    if (!bufSize)
    {
        snd_pcm_close(_pcmHandle);
        _pcmHandle=0;
        return 0;
    }

    initMixer(devName.replace("default", "hw").toStdString().c_str());

    return bufSize;
}

void alsaBackend::close()
{
    snd_pcm_close(_pcmHandle);
    _pcmHandle=0;

    if (_mixer)
    {
        snd_mixer_close(_mixer);
        _mixer=0;
    }

    free(_buffer);
}

size_t alsaBackend::setParams(unsigned int &sampleRate, const unsigned int channels, const unsigned int prec)
{
    int err;

    snd_pcm_hw_params_t *hwParams;
    snd_pcm_hw_params_alloca(&hwParams);

    err=snd_pcm_hw_params_any(_pcmHandle, hwParams);
    if (err<0)
    {
        qWarning() << "Error: " << snd_strerror(err);
        return 0;
    }

    err=snd_pcm_hw_params_set_access(_pcmHandle, hwParams, SND_PCM_ACCESS_RW_INTERLEAVED);
    if (err<0)
    {
        qWarning() << "Error: " << snd_strerror(err);
        return 0;
    }

    // Set sample format
    snd_pcm_format_t format;
    switch (prec)
    {
    case 1:
        format=SND_PCM_FORMAT_U8;
        break;
    case 2:
        format=SND_PCM_FORMAT_S16_LE;
        break;
    case 3:
        format=SND_PCM_FORMAT_S24_LE;
        break;
    case 4:
        format=SND_PCM_FORMAT_S32_LE;
        break;
    default:
        qWarning() << "Unsupported format";
        return 0;
    }

    err=snd_pcm_hw_params_set_format(_pcmHandle, hwParams, format);
    if (err<0)
    {
        qWarning() << "Error: " << snd_strerror(err);
        return 0;
    }

    // Set number of channels
    err=snd_pcm_hw_params_set_channels(_pcmHandle, hwParams, channels);
    if (err<0)
    {
        qWarning() << "Error: " << snd_strerror(err);
        return 0;
    }

    // Set samplerate
    err=snd_pcm_hw_params_set_rate_near(_pcmHandle, hwParams, (unsigned int*)&sampleRate, 0);
    if (err<0)
    {
        qWarning() <<  "Error: " << snd_strerror(err);
        return 0;
    }

    err=snd_pcm_hw_params(_pcmHandle, hwParams);
    if (err<0)
    {
        qWarning() << "Error: " << snd_strerror(err);
        return 0;
    }

    // Get buffer size
    snd_pcm_uframes_t bufSize;
    err=snd_pcm_hw_params_get_buffer_size(hwParams, &bufSize);
    if (err<0)
    {
        qWarning() << "Error: " << snd_strerror(err);
        return 0;
    }

    int bufferSize=snd_pcm_frames_to_bytes(_pcmHandle, bufSize);
    qDebug() << "Buffer size=" << bufferSize << " bytes";

    err=snd_pcm_prepare(_pcmHandle);
    if (err<0)
    {
        qWarning() << "Error: " << snd_strerror(err);
        return 0;
    }

    _buffer=malloc(bufferSize);

    return (_buffer)?bufferSize:0;
}

bool alsaBackend::write(void* buffer, size_t bufferSize)
{
    bufferSize=snd_pcm_bytes_to_frames(_pcmHandle, bufferSize);
loop:
    int n=snd_pcm_writei(_pcmHandle, buffer, bufferSize);

    if (n<0)
    {
        n=snd_pcm_recover(_pcmHandle, n, 0);
        if (n<0)
            return false;
        goto loop;
    }

    bufferSize-=n;
    if (!bufferSize)
        return true;

    buffer=(char*)buffer+snd_pcm_frames_to_bytes(_pcmHandle, n);
    goto loop;
}

void alsaBackend::stop()
{
    //snd_pcm_drain(_pcmHandle);
    snd_pcm_drop(_pcmHandle);
}

void alsaBackend::volume(int vol)
{
    if (!_mixer)
        return;

    if (snd_mixer_selem_set_playback_volume_all(_mixerHandle, vol)<0)
    {
        qDebug() <<"Error setting volume";
    }
}

int alsaBackend::volume()
{
    if (!_mixer)
        return -1;

    snd_mixer_handle_events(_mixer);

    long vol=-1;
    if (snd_mixer_selem_get_playback_volume(_mixerHandle, SND_MIXER_SCHN_MONO, &vol)<0) // SND_MIXER_SCHN_FRONT_RIGHT
    {
        qDebug() << "Error getting volume";
    }
    qDebug() << "alsa vol: " << vol;
    return (int)vol;
}

bool alsaBackend::initMixer(const char* dev)
{
    int err;
    long min, max;

    err=snd_mixer_open(&_mixer, 0);
    if (err<0)
        goto errorMsg;

    err=snd_mixer_attach(_mixer, dev);
    if (err<0)
        goto errorMsg;

    err=snd_mixer_selem_register(_mixer, 0, 0);
    if (err<0)
        goto errorMsg;

    err=snd_mixer_load(_mixer);
    if (err<0)
        goto errorMsg;

    snd_mixer_selem_id_t *selem_id;
    err=snd_mixer_selem_id_malloc(&selem_id);
    if (err<0)
        goto errorMsg;

    snd_mixer_selem_id_set_index(selem_id, 0);
    snd_mixer_selem_id_set_name(selem_id, "PCM");

    _mixerHandle=snd_mixer_find_selem(_mixer, selem_id);

    if (!snd_mixer_selem_has_playback_volume(_mixerHandle))
    {
        snd_mixer_selem_id_set_name(selem_id, "Wave");
        _mixerHandle=snd_mixer_find_selem(_mixer, selem_id);
        if (!snd_mixer_selem_has_playback_volume(_mixerHandle))
            _mixerHandle=0;
    }

    snd_mixer_selem_id_free(selem_id);

    if (!_mixerHandle)
        goto error;

    snd_mixer_selem_get_playback_volume_range(_mixerHandle, &min, &max);

    if (!max)
        goto error;

    snd_mixer_selem_set_playback_volume_range(_mixerHandle, 0, 100);

    snd_mixer_selem_get_playback_volume(_mixerHandle, SND_MIXER_SCHN_FRONT_LEFT, &min);
    snd_mixer_selem_get_playback_volume(_mixerHandle, SND_MIXER_SCHN_FRONT_RIGHT, &max);

    return true;

errorMsg:
    qWarning() << "Error: " << snd_strerror(err);
error:
    if (_mixer)
    {
        snd_mixer_close(_mixer);
        _mixer=0;
    }
    return false;
}

#endif
