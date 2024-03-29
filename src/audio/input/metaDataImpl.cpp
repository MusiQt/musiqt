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

#include "metaDataImpl.h"

#include <QDebug>

#define gettext(x) x

const char* metaDataImpl::mprisTags[LAST_ID] =
{
    gettext("trackId"),
    gettext("length"),
    gettext("artUrl"),
    gettext("album"),
    gettext("album artist"),
    gettext("artist"),
    gettext("lyrics"),
    gettext("BPM"),
    gettext("Rating"),
    gettext("comment"),
    gettext("composer"),
    gettext("year"),
    gettext("disc number"),
    gettext("first used"),
    gettext("genre"),
    gettext("last used"),
    gettext("lyricist"),
    gettext("title"),
    gettext("track"),
    gettext("URL"),
    gettext("user count"),
    gettext("user rating"),
};

void metaDataImpl::addInfo(QString type, QString info)
{
    if (!info.isEmpty())
    {
        qDebug().nospace() << "adding metadata: " << type << ", value: " << info;

        m_infos.insert(type, info.trimmed());
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
    i++;
    return (i < values.size()) ? i : -1;
}

QString metaDataImpl::getInfo(const char* info) const
{
    auto it = m_infos.find(info);
    return it != m_infos.end() ? it.value() : QString();
}
