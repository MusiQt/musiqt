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

#include "inputBackend.h"

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

class oggBackend : public inputBackend
{
private:
    OggVorbis_File *_vf;
    vorbis_info *_vi;

    QFile _file;

    oggConfig m_config;

private:
    static ov_callbacks vorbis_callbacks;

    static size_t read_func(void *ptr, size_t size, size_t nmemb, void *datasource);
    static int seek_func(void *datasource, ogg_int64_t offset, int whence);
    static int close_func(void *datasource);
    static long tell_func(void *datasource);

private:
    oggBackend();

public:
    ~oggBackend();

    static const char name[];

    /// Factory method
    static inputBackend* factory() { return new oggBackend(); }

    /// Get supported extension
    static QStringList ext();

    /// Open file
    bool open(const QString& fileName) override;

    /// Close file
    void close() override;

    /// Seek support
    bool seekable() const override { return true; }

    /// Seek specified position
    bool seek(int pos) override;

    /// Get samplerate
    unsigned int samplerate() const override { return _vi != nullptr ? _vi->rate : 0; }

    /// Get channels
    unsigned int channels() const override { return _vi != nullptr ? _vi->channels : 0; }

    /// Get precision
    sample_t precision() const override { return m_config.precision(); }

    /// Callback function
    size_t fillBuffer(void* buffer, const size_t bufferSize);

    /// Gapless support
    bool gapless() const override { return true; };

    // TODO remove

    /// Open config dialog
    QWidget* config(QWidget* win) override { return m_config.config(win); }

    void loadSettings() override { m_config.loadSettings(); }

    void saveSettings() override { m_config.saveSettings(); }
};

#endif
