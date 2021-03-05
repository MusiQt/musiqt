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

#include "inputBackend.h"

#include <QFile>

/*****************************************************************/

typedef struct
{
    bool fastscan;
    QString decoder;
} mpg123Config_t;

/*****************************************************************/

#include "configFrame.h"

class mpg123Config : public configFrame
{
    Q_OBJECT

private:
    mpg123Config() {}
    mpg123Config(const mpg123Config&);
    mpg123Config& operator=(const mpg123Config&);

private slots:
    void onCmdDecoder(int val);
    void onCmdFastscan(int val);

public:
    mpg123Config(QWidget* win);
    virtual ~mpg123Config() {}
};

/*****************************************************************/

class mpg123Backend final : public inputBackend
{
    friend class mpg123Config;

private:
    static mpg123Config_t _settings;

private:
    mpg123_handle *_handle;

    long _samplerate;
    int _channels;

    QFile _file;

    static int _status;

    static ssize_t read_func(void*, void*, size_t);
    static off_t seek_func(void*, off_t, int);

public:
    static QStringList _decoders;

private:
    mpg123Backend();

public:
    ~mpg123Backend();

    static const char name[];

    /// Factory method
    static inputBackend* factory() { return new mpg123Backend(); }

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
    unsigned int samplerate() const override { return _samplerate; }

    /// Get channels
    unsigned int channels() const override { return _channels; }

    /// Get precision
    sample_t precision() const override { return sample_t::S16; }

    /// Callback function
    size_t fillBuffer(void* buffer, const size_t bufferSize, const unsigned int seconds);

    /// Gapless support
    bool gapless() const override { return true; };

    /// Open config dialog
    QWidget* config(QWidget* win) override { return new mpg123Config(win); }

    void loadSettings() override;

    void saveSettings() override;
};

#endif
