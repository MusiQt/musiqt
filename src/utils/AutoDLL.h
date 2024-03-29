/*
 *  Copyright (C) 2009-2021 Leandro Nini
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

#ifndef AUTODLL_H
#define AUTODLL_H

#include <QLibrary>

class AutoDLL
{
private:
    QLibrary *m_handle;

private:
    AutoDLL(const AutoDLL&) = delete;
    AutoDLL &operator=(const AutoDLL&) = delete;

public:
    AutoDLL(const char* nm);

    ~AutoDLL();

    /// Check if library is loaded
    bool loaded() const { return m_handle != nullptr; }

    /// Return the address of the symbol in the library
    QFunctionPointer address(const char* sym) const;
};

#endif
