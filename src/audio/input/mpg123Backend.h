/*
 *  Copyright (C) 2009-2017 Leandro Nini
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
#include "AutoDLL.h"

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

    int _fd;

    static QFile _file[2];
    static int _count;

    static int _status;
    static const AutoDLL _dll;

    static ssize_t	read_func(int, void*, size_t);
    static off_t	seek_func(int, off_t, int);

public:
    static QStringList _decoders;

private:
    static int (*dl_mpg123_init)(void);
    static void (*dl_mpg123_exit)(void);
    static mpg123_handle* (*dl_mpg123_new)(const char*, int*);
    static void (*dl_mpg123_delete)(mpg123_handle*);
    static const char* (*dl_mpg123_plain_strerror)(int);
    static int (*dl_mpg123_open_fd)(mpg123_handle*, int);
    static int (*dl_mpg123_close)(mpg123_handle*);
    static int (*dl_mpg123_scan)(mpg123_handle*);
    static int (*dl_mpg123_getformat)(mpg123_handle*, long*, int*, int*);
    static off_t (*dl_mpg123_length)(mpg123_handle*);
    static int (*dl_mpg123_read)(mpg123_handle*, unsigned char*, size_t, size_t*);
    static off_t (*dl_mpg123_seek)(mpg123_handle*, off_t, int);
    static int (*dl_mpg123_id3)(mpg123_handle*, mpg123_id3v1**, mpg123_id3v2**);
    static const char** (*dl_mpg123_supported_decoders)(void);
    static int (*dl_mpg123_replace_reader)(mpg123_handle*, ssize_t(*)(int, void *, size_t), off_t(*)(int, off_t, int));
    static int (*dl_mpg123_param)(mpg123_handle*, enum mpg123_parms, long, double);

private:
    mpg123Backend();

public:
    ~mpg123Backend();

    static const char name[];

    static bool init();

    /// Factory method
    static input* factory() { return new mpg123Backend(); }

    /// Check if we support ext
    static bool supports(const QString& fileName);

    /// Get supported extension
    QStringList ext() const override;

    /// Open file
    bool open(const QString& fileName) override;

    /// Close file
    void close() override;

    /// Rewind to start
    bool rewind() override;

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

    void loadSettings();

    void saveSettings();
};

#endif
