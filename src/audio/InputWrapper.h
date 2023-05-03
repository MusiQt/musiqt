/*
 *  Copyright (C) 2020-2023 Leandro Nini
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

#ifndef INPUTWRAPPER_H
#define INPUTWRAPPER_H

#include <QIODevice>

#include "inputTypes.h"

class input;
class audioProcess;
class converter;

class InputWrapper : public QIODevice
{
    Q_OBJECT

signals:
    void updateTime();
    void preloadSong();
    void songFinished();

private:
    qint64 fillBuffer(char *data, qint64 maxSize);

protected:
    qint64 readData(char *data, qint64 maxSize) override;
    qint64 writeData(const char *data, qint64 maxSize) override;

#if QT_VERSION >= 0x060000
    // hack for Qt6 :/
    qint64 bytesAvailable() const override { return 16384 + QIODevice::bytesAvailable(); }
#endif

public:
    InputWrapper(input* song);
    ~InputWrapper();

    bool tryPreload(input* newSong);
    void unload();

    void enableBs2b();

    bool setFormat(audioFormat_t format);

    unsigned int getPosition() const { return m_milliSeconds; }

    // Set position [0,1]
    void setPosition(double pos);

private:
    input *m_currentSong;
    input *m_preloadedSong;

    audioProcess *m_audioProcess;

    converter *m_audioConverter;

    int m_bytes;
    int m_bytePerMilliSec;
    unsigned int m_milliSeconds;

    const unsigned int m_maxPlayTime;
};

#endif
