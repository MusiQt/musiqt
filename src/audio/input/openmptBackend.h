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

#ifndef OPENMPT_BACKEND_H
#define OPENMPT_BACKEND_H

#define LIBOPENMPT_ANCIENT_COMPILER
#include <libopenmpt/libopenmpt.hpp>

#include "inputBackend.h"

/*****************************************************************/

typedef struct
{
    int channels;
    int samplerate;
    int resamplingMode;
    int masterGain;
    int stereoSeparation;
    int volumeRamping;
} openmptConfig_t;

/*****************************************************************/

#include "configFrame.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>

class openmptConfig : public configFrame
{
private:
    openmptConfig() {}
    openmptConfig(const openmptConfig&);
    openmptConfig& operator=(const openmptConfig&);

public:
    openmptConfig(QWidget* win);
    virtual ~openmptConfig() {}
};

/*****************************************************************/

class openmptBackend : public inputBackend
{
    friend class openmptConfig;

private:
    static openmptConfig_t _settings;

private:
    openmpt::module *_module;

    static QStringList _ext;

private:
    openmptBackend();

    /// Get temp file name
    const QString tempFile(const QString& fileName)
    {
        QFileInfo fInfo(fileName);
        return QDir::tempPath() + QDir::separator() + fInfo.completeBaseName() + ".mod";
    }

    /// Delete temp file
    void delTempFile(bool del, const QString& tmpFile)
    {
        if (del)
        {
            qWarning() << "Deleting temp file: " << tmpFile;
            QFile::remove(tmpFile);
        }
    }

public:
    ~openmptBackend();

    static const char name[];

    static bool init();

    /// Factory method
    static inputBackend* factory() { return new openmptBackend(); }

    /// Get supported extension
    static QStringList ext() { return _ext; }

    /// Open file
    bool open(const QString& fileName) override;

    /// Close file
    void close() override;

    /// Rewind to start
    bool rewind() override;

    /// Number of subtunes
    unsigned int subtunes() const override;

    /// Current subtune
    unsigned int subtune() const override;

    /// Change subtune
    bool subtune(const unsigned int i) override;

    /// Get samplerate
    unsigned int samplerate() const override { return _settings.samplerate; }

    /// Get channels
    unsigned int channels() const override { return _settings.channels; }

    /// Get precision
    sample_t precision() const override { return sample_t::SAMPLE_FLOAT; }

    /// Callback function
    size_t fillBuffer(void* buffer, const size_t bufferSize) override;

    /// Open config dialog
    QWidget* config(QWidget* win) override { return new openmptConfig(win); }

    void loadSettings() override;

    void saveSettings() override;
};

#endif
