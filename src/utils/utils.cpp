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

#include "utils.h"

#include <cmath>

#include <QString>

#ifdef _WIN32
#include <windows.h>

#ifdef UNICODE
const wchar_t* utils::convertUtf(const QString& string)
{
    wchar_t* temp = new wchar_t[string.length()+1];
    int size = string.toWCharArray(temp);
    temp[size] = '\0';
    return temp;
}
#endif //UNICODE

#endif //_WIN32

constexpr int MAX_CHARS = 20;

QString utils::shrink(const QString& string)
{
    return (string.length() > MAX_CHARS)
        ? QString("...%1").arg(string.right(MAX_CHARS))
        : string;
}
