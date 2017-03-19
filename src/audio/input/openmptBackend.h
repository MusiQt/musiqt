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
    Q_OBJECT

private:
    openmptConfig() {}
    openmptConfig(const openmptConfig&);
    openmptConfig& operator=(const openmptConfig&);

public:
    openmptConfig(QWidget* win);
    virtual ~openmptConfig() {}

private slots:
    void onCmdFrequency(int val);
    void onCmdChannels(int val);
    void onCmdResampling(int val);

    void onCmdMasterGain(int val);
    void onCmdStereoSeparation(int val);
    void onCmdVolumeRamping(int val);
};

/*****************************************************************/

class openmptBackend : public inputBackend
{
    friend class openmptConfig;

private:
    static openmptConfig_t _settings;

private:
    openmpt::module *_module;

    bool _unmo3;

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
    static input* factory() { return new openmptBackend(); }

    /// Check if we support ext
    static bool supports(const QString& fileName);

    /// Get supported extension
    QStringList ext() const override { return _ext; }

    /// Open file
    bool open(const QString& fileName) override;

    /// Close file
    void close() override;

    /// Rewind to start
    bool rewind() override;

    /// Get samplerate
    unsigned int samplerate() const override { return _settings.samplerate; }

    /// Get channels
    unsigned int channels() const override { return _settings.channels; }

    /// Get precision
    sample_t precision() const override { return SAMPLE_FLOAT; }

    /// Callback function
    size_t fillBuffer(void* buffer, const size_t bufferSize, const unsigned int seconds) override;

    /// Open config dialog
    QWidget* config(QWidget* win) override { return new openmptConfig(win); }

    void loadSettings();

    void saveSettings();
};

#endif
