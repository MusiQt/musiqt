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

#ifndef MPEG_H
#define MPEG_H

#include <mpg123.h>

#include "input.h"

#include <QFile>

/*****************************************************************/

typedef struct
{
    bool fastscan;
    QString decoder;
} mpg123Config_t;

/*****************************************************************/

#include "configFrame.h"

class mpg123ConfigFrame : public configFrame
{
private:
    mpg123ConfigFrame() {}
    mpg123ConfigFrame(const mpg123ConfigFrame&);
    mpg123ConfigFrame& operator=(const mpg123ConfigFrame&);

public:
    mpg123ConfigFrame(QWidget* win);
    virtual ~mpg123ConfigFrame() {}
};

/*****************************************************************/

class mpg123Config : public inputConfig
{
    friend class mpg123ConfigFrame;

private:
    static mpg123Config_t m_settings;

public:
    mpg123Config(const char name[], const unsigned char* iconType, unsigned int iconLen) :
        inputConfig(name, iconType, iconLen)
    {
        loadSettings();
    }

    /// Open config dialog
    QWidget* config(QWidget* win) override { return new mpg123ConfigFrame(win); }

    bool fastscan() const { return m_settings.fastscan; }

    QString decoder() const { return m_settings.decoder; }

    void loadSettings() override;

    void saveSettings() override;
};

/*****************************************************************/

class mpg123Backend final : public input
{
private:
    mpg123_handle *m_handle;

    long m_samplerate;
    int m_channels;

    QFile m_file;

    static ssize_t read_func(void*, void*, size_t);
    static off_t seek_func(void*, off_t, int);

    mpg123Config m_config;

public:
    static QStringList m_decoders;

private:
    mpg123Backend(const QString& fileName);

public:
    ~mpg123Backend();

    static const char name[];

    static bool init();

    /// Factory method
    static input* factory(const QString& fileName) { return new mpg123Backend(fileName); }
    static inputConfig* cFactory();

    /// Get supported extension
    static QStringList ext();

    /// Seek support
    bool seekable() const override { return true; }

    /// Seek specified position
    bool seek(int pos) override;

    /// Get samplerate
    unsigned int samplerate() const override { return m_samplerate; }

    /// Get channels
    unsigned int channels() const override { return m_channels; }

    /// Get precision
    sample_t precision() const override { return sample_t::S16; }

    /// Callback function
    size_t fillBuffer(void* buffer, const size_t bufferSize);

    /// Gapless support
    bool gapless() const override { return true; };
};

#endif
