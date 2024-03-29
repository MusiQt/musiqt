/*
 *  Copyright (C) 2008-2023 Leandro Nini
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

#include "iconFactory.h"

#include "settings.h"

#include <QFileInfo>

iconFactory* iconFactory::instance()
{
    static iconFactory i;
    return &i;
}

void iconFactory::destroy()
{
    m_icons.clear();
}

QIcon iconFactory::get(const char* icon)
{
    if (icon == nullptr)
        return QIcon();

    if (SETTINGS->themeIcons()) {
        QFileInfo fi(icon);
        if (QIcon::hasThemeIcon(fi.baseName())) {
            QIcon themed = QIcon::fromTheme(fi.baseName());
            return themed;
        }
    }

    QHash<const char*, QIcon>::iterator i = m_icons.find(icon);
    if (i == m_icons.end())
    {
        i = m_icons.insert(icon, QIcon(icon));
    }
    return i.value();
}
