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

//const char* convertNc(const wchar_t* string)
//{
//	char* temp = new char[1];
//    //TODO
//	return string.utf16();
//}
#endif //UNICODE

#endif //_WIN32

#if 0 // unused
void utils::runCmd(const QString& cmd)
{
#ifdef _WIN32
    PROCESS_INFORMATION pi = {0};
    STARTUPINFO si = {sizeof(si)};
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

#ifdef UNICODE
    wchar_t* command = const_cast<wchar_t*>(convertUtf(cmd));
#else
    char* command = cmd.toLocal8Bit().data();
#endif //UNICODE

    if (CreateProcess(0, command, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
    {
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
#ifdef UNICODE
    delete [] command;
#endif //UNICODE
#else
    QByteArray c = cmd.toLocal8Bit();
    system(c.constData());
#endif //_WIN32
}
#endif

#define MAX_CHARS	20

QString utils::shrink(const QString& string)
{
    return (string.count() > MAX_CHARS)
        ? QString("...%1").arg(string.right(MAX_CHARS))
        : string;
}

#if 0 // unused
/// Get gray value from RGBA color
#define GRAYVAL(rgba) (30*rgba.red() + 59*rgba.green() + 11*rgba.blue())/100

#define CLAMP(min, x, max) (x<min ? min : (x>max ? max : x))

QColor utils::altColor(QColor color)
{
    int val = GRAYVAL(color);
    val = static_cast<int>(((val<128)?15.0:-15.0)*exp((255-val)/196.0));

    return QColor(CLAMP(0, color.red()+val, 255),
        CLAMP(0, color.green()+val, 255),
        CLAMP(0, color.blue()+val, 255));
}
#endif
