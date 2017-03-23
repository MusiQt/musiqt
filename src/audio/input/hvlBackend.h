/*
 *  Copyright (C) 2007-2017 Leandro Nini
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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef HVL_H
#define HVL_H

extern "C"{
#include "libs/hvl_replay/hvl_replay.h"
}

#include "inputBackend.h"

/*****************************************************************/

typedef struct
{
    int samplerate;
} hvlConfig_t;

/*****************************************************************/

#include "configFrame.h"

class hvlConfig : public configFrame
{
    Q_OBJECT

private:
    hvlConfig() {}
    hvlConfig(const hvlConfig&);
    hvlConfig& operator=(const hvlConfig&);

public:
    hvlConfig(QWidget* win);
    virtual ~hvlConfig() {}

private slots:
    void onCmdSamplerate(int val);
};

/*****************************************************************/

class hvlBackend : public inputBackend
{
    friend class hvlConfig;

private:
    static hvlConfig_t _settings;

private:
    struct hvl_tune *_tune;

    int _subtune;

    int _left;
    unsigned int _size;
    char *_buffer;

private:
    hvlBackend();

public:
    ~hvlBackend();

    static const char name[];

    /// Factory method
    static input* factory() { return new hvlBackend(); }

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

    /// Get number of subtunes
    unsigned int subtunes() const override { return _tune ? _tune->ht_SubsongNr : 0; }

    /// Get current subtune
    unsigned int subtune() const override { return _tune ? _tune->ht_SongNum : 0; }

    /// Change subtune
    bool subtune(const unsigned int i) override;

    /// Get samplerate
    unsigned int samplerate() const override { return _settings.samplerate; }

    /// Get channels
    unsigned int channels() const override { return 2; }

    /// Get precision
    sample_t precision() const override { return sample_t::S16; }

    /// Callback function
    size_t fillBuffer(void* buffer, const size_t bufferSize, const unsigned int seconds) override;

    /// Open config dialog
    QWidget* config(QWidget* win) override { return new hvlConfig(win); }

    void loadSettings();

    void saveSettings();
};

#endif
