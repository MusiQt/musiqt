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

#include "InputWrapper.h"

#include "input/input.h"

#include <QDebug>

InputWrapper::InputWrapper(input* i) :
    _input(i),
    _preload(nullptr),
    seconds(0)
{}

qint64 InputWrapper::readData(char *data, qint64 maxSize)
{
    if (maxSize == 0)
    {
        qDebug() << "readData maxSize=0";
        return 0;
    }
    size_t n = _input->fillBuffer((void*)data, maxSize, seconds);
    if (n == 0)
    {
        emit songEnded();
        qDebug() << "***";
        if (_preload != nullptr)
        {
            _input = _preload;
            _preload = nullptr;
            seconds = 0;
            //n = _input->fillBuffer((void*)data, maxSize, seconds);
            return 0;
        }
        else
            return -1;
    }

    bytes += n;
    if (bytes > bytePerSec)
    {
        do {
            bytes -= bytePerSec;
            seconds++;
        } while (bytes > bytePerSec);
        if (seconds != _input->time()-5)
            emit updateTime();
        else
            emit preloadSong();
    }

    return n;
}

qint64 InputWrapper::writeData(const char *data, qint64 maxSize) { return -1; }

bool InputWrapper::tryPreload(input* i)
{
    if ((_input->samplerate() == i->samplerate())
        && (_input->channels() == i->channels())
        && (_input->precision() == i->precision()))
    {
        _preload = i;
        return true;
    }
    else
    {
        _preload = nullptr;
        return false;
    }
}

void InputWrapper::unload()
{
    _preload = nullptr;
}

void InputWrapper::setBPS(int bps)
{
    bytePerSec = bps;
    bytes = 0;
}
