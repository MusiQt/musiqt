/*
 *  Copyright (C) 2011-2019 Leandro Nini
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

#include <QtGlobal>
#include <QDesktopServices>
#include <QUrl>

#if QT_VERSION >= 0x050000
#  include <QStandardPaths>
#else
#  include <cstdlib>

#  include <QDir>
#  include <QDebug>

#  ifdef _WIN32
#    include <windows.h>
#    include <shlobj.h>
#  endif
#endif

#if defined(_WIN32) && QT_VERSION < 0x050000
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
#if QT_VERSION >= 0x050000
    return QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
#else
#  ifdef _WIN32
    return getWindowsDir(CSIDL_INTERNET_CACHE);
#  else
    QString xdgCacheDir(qgetenv("XDG_CACHE_HOME"));
    if (xdgCacheDir.isEmpty())
        xdgCacheDir = QDir::homePath()+"/.cache";
    return QDir::cleanPath(xdgCacheDir);
#  endif
#endif
}

const QString xdg::getRuntimeDir()
{
#if QT_VERSION >= 0x050000
    return QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation);
#else
#  ifndef _WIN32
    QString xdgRuntimeDir(qgetenv("XDG_RUNTIME_DIR"));
    if (!xdgRuntimeDir.isEmpty())
        return QDir::cleanPath(xdgRuntimeDir);
#  endif
    return getCacheDir();
#endif
}

const QString xdg::getConfigDir()
{
#if QT_VERSION >= 0x050000
    return QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
#else
#  ifdef _WIN32
    return getWindowsDir(CSIDL_APPDATA);
#  else
    QString xdgConfigDir(qgetenv("XDG_CONFIG_HOME"));
    if (xdgConfigDir.isEmpty())
        xdgConfigDir = QDir::homePath()+"/.config";
    return QDir::cleanPath(xdgConfigDir);
#  endif
#endif
}

const QString xdg::getMusicDir()
{
#if QT_VERSION >= 0x050000
    return QStandardPaths::writableLocation(QStandardPaths::MusicLocation);
#else
#  ifdef _WIN32
    return getWindowsDir(CSIDL_MYMUSIC);
#  else
    QString xdgUserDirs=getConfigDir();
    xdgUserDirs.append("/user-dirs.dirs");
    qDebug() << "xdgUserDirs: " << xdgUserDirs;

    QString musicDir;
    QFile userDirs(xdgUserDirs);
    if (userDirs.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&userDirs);
        while (!in.atEnd())
        {
            QString line = in.readLine();
            qDebug() << "user-dirs.dirs: " << line;
            if (!line.compare("XDG_MUSIC_DIR"))
                musicDir.append(line.replace('"', ' ').trimmed().midRef(line.indexOf('=')));
        }
    }

    return QDir::cleanPath(musicDir);
#  endif
#endif
}

bool xdg::open(const QString& link)
{
    return QDesktopServices::openUrl(QUrl(link));
}
