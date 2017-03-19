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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "trackListFactory.h"

#include "trackListDir.h"
#include "trackListPls.h"
#include "trackListM3u.h"
#ifdef HAVE_LIBCUE
#  include "trackListCue.h"
#endif

tFactory* tFactory::instance()
{
    static tFactory o;
    return &o;
}

trackList* tFactory::get(const QString &path)
{
    if (QDir(path).exists())
        return new trackListDir(path);

    QFileInfo file(path);
    const QString ext=file.suffix();
    if (!QString::compare(ext, "pls"))
        return new trackListPls(path);

    if (!QString::compare(ext, "m3u"))
        return new trackListM3u(path);

#ifdef HAVE_LIBCUE
    if (!QString::compare(ext, "cue"))
        return new trackListCue(path);
#endif

    return nullptr;
}
