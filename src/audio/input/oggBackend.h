/*
 *  Copyright (C) 2006-2021 Leandro Nini
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

#ifndef OGG_H
#define OGG_H

#include "input.h"

#include <vorbis/vorbisfile.h>

#include <QFile>

/*****************************************************************/

typedef struct
{
    sample_t precision;
} oggConfig_t;

/*****************************************************************/

#include "configFrame.h"

class oggConfigFrame : public configFrame
{
private:
    oggConfigFrame() {}
    oggConfigFrame(const oggConfigFrame&);
    oggConfigFrame& operator=(const oggConfigFrame&);

public:
    oggConfigFrame(QWidget* win);
    virtual ~oggConfigFrame() {}
};

/*****************************************************************/

class oggConfig : public inputConfig
{
    friend class oggConfigFrame;

private:
    static oggConfig_t m_settings;

public:
    oggConfig(const char name[], const unsigned char* iconType, unsigned int iconLen) :
        inputConfig(name, iconType, iconLen)
    {
        loadSettings();
    }

    /// Open config dialog
    QWidget* config(QWidget* win) override { return new oggConfigFrame(win); }

    sample_t precision() const { return m_settings.precision; }

    void loadSettings() override;

    void saveSettings() override;
};

/*****************************************************************/

class oggBackend : public input
{
private:
    OggVorbis_File *m_vf;

    QFile m_file;
    
    unsigned int m_samplerate;
    unsigned int m_channels;

    oggConfig m_config;

private:
    static ov_callbacks vorbis_callbacks;

    static size_t read_func(void *ptr, size_t size, size_t nmemb, void *datasource);
    static int seek_func(void *datasource, ogg_int64_t offset, int whence);
    static int close_func(void *datasource);
    static long tell_func(void *datasource);

private:
    oggBackend(const QString& fileName);

public:
    ~oggBackend();

    static const char name[];

    /// Factory method
    static input* factory(const QString& fileName) { return new oggBackend(fileName); }
    static inputConfig* cFactory();

    /// Get supported extension
    static QStringList ext();

    /// Seek support
    bool seekable() const override { return true; }

    /// Seek specified position
    bool seek(double pos) override;

    /// Get samplerate
    unsigned int samplerate() const override { return m_samplerate; }

    /// Get channels
    unsigned int channels() const override { return m_channels; }

    /// Get precision
    sample_t precision() const override { return m_config.precision(); }

    /// Callback function
    size_t fillBuffer(void* buffer, const size_t bufferSize);

    /// Gapless support
    bool gapless() const override { return true; };
};

#endif
