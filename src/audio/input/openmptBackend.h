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

#include "input.h"

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

class openmptConfigFrame : public configFrame
{
private:
    openmptConfigFrame() {}
    openmptConfigFrame(const openmptConfigFrame&);
    openmptConfigFrame& operator=(const openmptConfigFrame&);

public:
    openmptConfigFrame(QWidget* win);
    virtual ~openmptConfigFrame() {}
};

/*****************************************************************/

class openmptConfig : public inputConfig
{
    friend class openmptConfigFrame;

private:
    static openmptConfig_t m_settings;

public:
    openmptConfig(const char name[], const unsigned char* iconType, unsigned int iconLen) :
        inputConfig(name, iconType, iconLen)
    {
        loadSettings();
    }

    /// Open config dialog
    QWidget* config(QWidget* win) override { return new openmptConfigFrame(win); }

    int channels() const { return m_settings.channels; }

    int samplerate() const { return m_settings.samplerate; }

    int resamplingMode() const { return m_settings.resamplingMode; }

    int masterGain() const { return m_settings.masterGain; }

    int stereoSeparation() const { return m_settings.stereoSeparation; }

    int volumeRamping() const { return m_settings.volumeRamping; }

    void loadSettings() override;

    void saveSettings() override;
};

/*****************************************************************/

class openmptBackend : public input
{
private:
    openmpt::module *m_module;

    static QStringList m_ext;

    openmptConfig m_config;

private:
    openmptBackend(const QString& fileName);

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
    static input* factory(const QString& fileName) { return new openmptBackend(fileName); }
    static inputConfig* cFactory();

    /// Get supported extension
    static QStringList ext() { return m_ext; }

    /// Rewind to start
    bool rewind() override;

    /// Number of subtunes
    unsigned int subtunes() const override { return m_module->get_num_subsongs(); }

    /// Current subtune
    unsigned int subtune() const override { return m_module->get_selected_subsong(); }

    /// Change subtune
    bool subtune(const unsigned int i) override;

    /// Get samplerate
    unsigned int samplerate() const override { return m_config.samplerate(); }

    /// Get channels
    unsigned int channels() const override { return m_config.channels(); }

    /// Get precision
    sample_t precision() const override { return sample_t::SAMPLE_FLOAT; }

    /// Callback function
    size_t fillBuffer(void* buffer, const size_t bufferSize) override;
};

#endif
