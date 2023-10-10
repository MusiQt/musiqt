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

#include <QDebug>

AutoDLL::AutoDLL(const char* nm) :
    m_handle(nullptr)
{
    if (QLibrary::isLibrary(nm))
    {
        m_handle = new QLibrary(nm);
        if (!m_handle->load())
        {
            qWarning() << m_handle->errorString();
            utils::delPtr(m_handle);
        }
    }
}

AutoDLL::~AutoDLL()
{
    if (m_handle != nullptr)
    {
        m_handle->unload();
        utils::delPtr(m_handle);
    }
}

QFunctionPointer AutoDLL::address(const char* sym) const
{
    if (m_handle != nullptr)
        return m_handle->resolve(sym);

    return nullptr;
}
