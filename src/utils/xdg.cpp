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

#include <QDesktopServices>
#include <QUrl>

#if QT_VERSION >= 0x050000
#  include <QStandardPaths>
#endif

const QString xdg::getCacheDir()
{
#if QT_VERSION >= 0x050000
    return QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
#else
    return QDesktopServices::storageLocation(QDesktopServices::CacheLocation);
#endif
}

const QString xdg::getRuntimeDir()
{
#if QT_VERSION >= 0x050000
    return QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation);
#else
    return QDesktopServices::storageLocation(QDesktopServices::CacheLocation);
#endif
}

const QString xdg::getConfigDir()
{
#if QT_VERSION >= 0x050000
    return QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
#else
    return QDesktopServices::storageLocation(QDesktopServices::DataLocation);
#endif
}

const QString xdg::getMusicDir()
{
#if QT_VERSION >= 0x050000
    return QStandardPaths::writableLocation(QStandardPaths::MusicLocation);
#else
    return QDesktopServices::storageLocation(QDesktopServices::MusicLocation);
#endif
}

bool xdg::open(const QString& link)
{
    return QDesktopServices::openUrl(QUrl(link));
}
