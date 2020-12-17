/*
 *  Copyright (C) 2020 Leandro Nini
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

class input;

class InputWrapper : public QIODevice
{
    Q_OBJECT

signals:
    void songEnded();
    void updateTime();
    void preloadSong();

public:
    InputWrapper(input* i);
    //~InputWrapper();

    qint64 readData(char *data, qint64 maxSize) override;
    qint64 writeData(const char *data, qint64 maxSize) override;

    //bool isSequential() const override;
    //qint64 bytesAvailable() const override;
    
    bool tryPreload(input* i);
    void unload();

    void setBPS(int size);
    
    int getSeconds() const { return seconds; }

private:
    input *_input;
    input *_preload;

    int bytes;
    int bytePerSec;
    int seconds;
};

#endif
