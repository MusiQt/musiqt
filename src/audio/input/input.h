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

#ifndef INPUT_H
#define INPUT_H

#include "inputTypes.h"
#include "metaData.h"

#include <QIcon>
#include <QString>
#include <QWidget>

/**
 * interface class for input config
 */
class inputConfig
{
public:
    virtual ~inputConfig() {}

    /// Open config dialog
    virtual QWidget* config(QWidget* parent=nullptr) =0;

    /// Get filetype icon
    virtual QIcon icon() const =0;

    /// Get Music directory
    virtual const QString getMusicDir() const =0;

    virtual void loadSettings() =0;

    virtual void saveSettings() =0;
};

/**
 * interface class for input backends
 */
class input
{
public:
    virtual ~input() {}

    /// Get song info
    virtual const metaData* getMetaData() const =0;

    /// Get song duration
    virtual unsigned int time() const =0;

    /// Get samplerate
    virtual unsigned int samplerate() const =0;

    /// Get channels
    virtual unsigned int channels() const =0;

    /// Get precision
    virtual sample_t precision() const =0;

    /// Get fractional scale for fixed point types
    virtual unsigned int fract() const =0;

    /// Open file
    virtual bool open(const QString& fileName) =0;

    /// Close file
    virtual void close() =0;

    /// Rewind to start
    virtual bool rewind() =0;

    /// Seek specified position
    virtual bool seek(const int pos) =0;

    /// Callback function
    virtual size_t fillBuffer(void* buffer, const size_t bufferSize, const unsigned int seconds) =0;

    /// Song is loaded
    virtual QString songLoaded() const =0;

    /// Get number of subtunes
    virtual unsigned int subtunes() const =0;

    /// Get current subtune
    virtual unsigned int subtune() const =0;

    /// Change subtune
    virtual bool subtune(const unsigned int i) =0;

    /// Gapless support
    virtual bool gapless() const =0;
};

#endif
