/*
 *  Copyright (C) 2023 Leandro Nini
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

#ifndef SYSPATHS_H
#define SYSPATHS_H

#include <QString>

#include "xdg.h"

/**
 * This is an utility class to deal with app dirs.
 */
namespace syspaths
{
    /// Get the directory for user specific non-essential data files
    const QString getStateDir();

    /// Get the directory for user-specific non-essential runtime files
    /// and other file objects (such as sockets, named pipes, ...)
    const QString getRuntimeDir();

    /// Get the directory relative to which user specific configuration
    /// files should be stored
    //const QString getConfigDir();
};

#endif
