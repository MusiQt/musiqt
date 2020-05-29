/*
 *  Copyright (C) 2010-2020 Leandro Nini
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

#include "infoLabel.h"

#include "input/metaData.h"

#include <QFileInfo>

#define APPENDINFO(info) \
temp = data->getInfo(info); \
if (!temp.isEmpty()) { \
    if (!tip.isEmpty() && tip[tip.length()-1]!='\n') \
        tip.append('\n'); \
    tip.append(QString("%1: %2").arg((data->getKey(info)), data->getInfo(info))); \
}

void infoLabel::setInfo(const metaData *data)
{
    QString tip;
    QString label;
    if (data != nullptr)
    {
        QString temp = data->getInfo(metaData::TITLE);
        if (!temp.isEmpty())
        {
            label = temp;

            temp = data->getInfo(metaData::ARTIST);
            if (!temp.isEmpty())
            {
                label.append(QString(" [%1]").arg(temp));
            }
        }
        else
        {
            QFileInfo fi(data->getInfo(metaData::LOCATION));
            label = fi.fileName();
        }
        APPENDINFO(metaData::TITLE);
        APPENDINFO(metaData::ARTIST);
        APPENDINFO(metaData::ALBUM);
        APPENDINFO(metaData::YEAR);
        APPENDINFO(metaData::TRACK);
        if (tip.isEmpty())
            tip = tr("No info");
    }

    setText(label);
    setToolTip(tip);

    update();
}
