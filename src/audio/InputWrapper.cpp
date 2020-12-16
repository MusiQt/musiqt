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

InputWrapper::InputWrapper(input* i) :
    _input(i),
    seconds(0)
{}

qint64 InputWrapper::readData(char *data, qint64 maxSize) {
    size_t n = _input->fillBuffer((void*)data, maxSize, 0);
    if (n == 0) {
        emit songEnded();
        seconds = 0;
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

void InputWrapper::setBPS(int bps) {
    bytePerSec = bps;
    bytes = 0;
}
