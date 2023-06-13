/*
 *  Copyright (C) 2011-2023 Leandro Nini
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

#include "xdg.h"

#include <QDesktopServices>
#include <QUrl>
#include <QDebug>
#include <QStandardPaths>
#ifndef _WIN32
#  include <QDir>
#endif

#ifdef _WIN32
#  include <windows.h>
#  include <shlobj.h>

#  ifdef UNICODE
#    define TCHAR2QString(x) QString::fromWCharArray(x)
#  else
#    define TCHAR2QString(x) QString::fromLocal8Bit(x)
#  endif

const QString getWindowsDir(const int csidl)
{
    TCHAR szPath[MAX_PATH];

    if (SHGetSpecialFolderPath(NULL, szPath, csidl, 1))
        return TCHAR2QString(szPath);
    else
        return QString();
}
#endif

const QString xdg::getCacheDir()
{
    return QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
}

const QString xdg::getRuntimeDir()
{
    return QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation);
}

const QString xdg::getConfigDir()
{
    return QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
}

const QString xdg::getStateDir()
{
#ifdef _WIN32
    return getWindowsDir(CSIDL_COMMON_APPDATA);
#else
    QString xdgStateDir(qgetenv("XDG_STATE_HOME"));
    if (xdgStateDir.isEmpty())
        xdgStateDir = QDir::homePath()+"/.local/state";
    return QDir::cleanPath(xdgStateDir);
#endif
}

const QString xdg::getMusicDir()
{
    QStringList locations = QStandardPaths::standardLocations(QStandardPaths::MusicLocation);
    // TODO return all music locations
    return (!locations.isEmpty()) ? locations.first() : QString();
}

bool xdg::open(const QString& link)
{
    return QDesktopServices::openUrl(QUrl(link));
}
