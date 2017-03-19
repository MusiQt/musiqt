/*
 *  Copyright (C) 2009-2017 Leandro Nini
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

#include "AutoDLL.h"

#include "utils.h"

AutoDLL::AutoDLL(const char* nm) :
    _handle(nullptr)
{
    if (QLibrary::isLibrary(nm))
    {
        _handle = new QLibrary(nm);
        if (!_handle->load())
            delPtr(_handle);
    }
}

AutoDLL::~AutoDLL(){
    if (_handle != nullptr)
    {
        _handle->unload();
        delPtr(_handle);
    }
}

QFunctionPointer AutoDLL::address(const char* sym) const
{
    if (_handle != nullptr)
        return _handle->resolve(sym);

    return nullptr;
}
