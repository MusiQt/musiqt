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

#ifndef TFACTORY_H
#define TFACTORY_H

#include "trackList.h"

#include <QStringList>

#define TFACTORY tFactory::instance()

#ifdef HAVE_LIBCUE
#  define EXT "pls,m3u,cue"
#else
#  define EXT "pls,m3u"
#endif

class tFactory
{
protected:
    tFactory() {}
    tFactory(const tFactory&);
    tFactory& operator= (const tFactory&);
    ~tFactory() {}

public:
    /// Get singleton instance
    static tFactory* instance();

    /// Instantiate backend
    trackList* get(const QString &path);

    /// Return supported playlist extensions
    const QStringList plExt() const { return QString(EXT).split(","); }
};

#endif
