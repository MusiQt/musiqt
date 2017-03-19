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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef TTA_H
#define TTA_H

#include "libs/libtta/libtta.h"

#include "inputBackend.h"

#include <stdio.h>

#include <QFile>

/*****************************************************************/

#include "configFrame.h"

class ttaConfig : public configFrame
{
    Q_OBJECT

private:
    ttaConfig() {}
    ttaConfig(const ttaConfig&);
    ttaConfig& operator=(const ttaConfig&);

public:
    ttaConfig(QWidget* win);
    virtual ~ttaConfig() {}
};

/*****************************************************************/

class ttaBackend : public inputBackend
{
private:
    typedef struct
    {
        TTA_io_callback iocb;
        QFile *handle;
    } tta_wrapper;

private:
    QFile _file;

    tta::tta_decoder *TTA;
    tta_wrapper io;
    TTAuint32 _sampleSize;
    TTAuint8 *_decodeBuf;
    TTAuint32 _bufOffset;
    TTAuint32 _bufSize;
    TTA_info _info;

private:
    static TTAint32 CALLBACK read_callback(TTA_io_callback *io, TTAuint8 *buffer, TTAuint32 size);
    static TTAint64 CALLBACK seek_callback(TTA_io_callback *io, TTAint64 offset);

    static const char* getError(const TTA_CODEC_STATUS e);

private:
    ttaBackend();

public:
    ~ttaBackend();

    static const char name[];

    /// Factory method
    static input* factory() { return new ttaBackend(); }

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
    unsigned int samplerate() const override
    {
        return !songLoaded().isNull()
            ? _info.sps
            : 0;
    }

    /// Get channels
    unsigned int channels() const override
    {
        return !songLoaded().isNull()
            ? _info.nch
            : 0;
    }

    /// Get precision
    sample_t precision() const override
    {
        return !songLoaded().isNull()
            ? (_info.bps == 16) ? S16 : U8
            : S16;
    }

    /// Callback function
    size_t fillBuffer(void* buffer, const size_t bufferSize, const unsigned int seconds) override;

    /// Gapless support
    bool gapless() const override { return true; } //check

    /// Open config dialog
    QWidget* config(QWidget* win) override { return new ttaConfig(win); }
};

#endif
