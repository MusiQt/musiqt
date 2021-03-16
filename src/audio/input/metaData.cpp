/*
 *  Copyright (C) 2010-2021 Leandro Nini
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

#include "metaData.h"

#include <QDebug>

#define gettext(x) x

const char* metaDataImpl::mprisTags[LAST_ID] =
{
    gettext("location"), // file location
    gettext("title"),
    gettext("artist"),
    gettext("album"),
    gettext("track"),    //tracknumber
    gettext("time"),
    gettext("mtime"),
    gettext("genre"),
    gettext("comment"),
    gettext("rating"),
    gettext("year"),
    gettext("arturl"),
    gettext("lyrics")
};

void metaDataImpl::addInfo(QString type, QString info)
{
    qDebug() << "adding metadata: " << type << ", value: "<< info;

    if (!info.isEmpty())
    {
        m_infos.remove(type);
        m_infos.insert(type, info);
    }
}

void metaDataImpl::addInfo(QString type, unsigned int info)
{
    addInfo(type, QString::number(info));
}

void metaDataImpl::addInfo(const mpris_t type, QString info)
{
    if (!info.isEmpty())
    {
        addInfo(mprisTags[type], info);
    }
}

void metaDataImpl::addInfo(const mpris_t type, unsigned int info)
{
    addInfo(type, QString::number(info));
}

int metaDataImpl::moreInfo(int i) const
{
    auto values = m_infos.values();
    i += 1;
    return (i < values.size()) ? i : -1;
}
