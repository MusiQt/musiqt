/*
 *  Copyright (C) 2008-2021 Leandro Nini
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

#ifndef ICONFACTORY_H
#define ICONFACTORY_H

#include "icons.h"

#include <QHash>
#include <QIcon>

#define	GET_ICON(icon) iconFactory::instance()->get(icon)

class iconFactory
{
private:
    QHash<const char*, QIcon> m_icons;

protected:
    iconFactory() {}
    iconFactory(const iconFactory&) = delete;
    iconFactory& operator= (const iconFactory&) = delete;
    ~iconFactory() = default;

public:
    /// Get singleton instance
    static iconFactory* instance();

    /// Delete all icons
    void destroy();

    /// Get icon
    QIcon get(const char* icon);
};

#endif
