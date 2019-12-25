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

#ifndef XDG_H
#define XDG_H

#include <QString>

/**
 * This is an utility class to deal with standard and user dirs.
 */
namespace xdg
{
    /// Get the base directory relative to which user specific non-essential
    /// data files should be stored
    const QString getCacheDir();

    /// Get the base directory relative to which user-specific non-essential
    /// runtime files and other file objects (such as sockets, named pipes,
    /// ...) should be stored
    const QString getRuntimeDir();

    /// Get the base directory relative to which user specific configuration
    /// files should be stored
    const QString getConfigDir();

    /// Get the user music directory
    const QString getMusicDir();

    /// Open a file or URL in the user's preferred application
    bool open(const QString& link);
}

#endif
