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

#include "input/input.h"

#include <QIODevice>

class InputWrapper : public QIODevice
{
public:
    InputWrapper(input* i);
    //~InputWrapper();

    qint64 readData(char *data, qint64 maxSize) override;
    qint64 writeData(const char *data, qint64 maxSize) override;

    //bool isSequential() const override;
    //qint64 bytesAvailable() const override;
    //
    //void init(qint64 size);

private:
    input *_input;
};

InputWrapper::InputWrapper(input* i) : _input(i) {}

qint64 InputWrapper::readData(char *data, qint64 maxSize) {
    return _input->fillBuffer((void*)data, maxSize, 0);
}

qint64 InputWrapper::writeData(const char *data, qint64 maxSize) { return -1; }



#endif
