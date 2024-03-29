/*
 *  Copyright (C) 2010-2017 Leandro Nini
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

#ifndef INFOLABEL_H
#define INFOLABEL_H

#include <QLabel>

class metaData;

class infoLabel final : public QLabel
{
private:
    infoLabel() {}
    infoLabel(const infoLabel&) = delete;
    infoLabel &operator=(const infoLabel&) = delete;

public:
    infoLabel(QWidget* p) :
        QLabel(p) {}

    void setInfo(const metaData *data);

     QSize sizeHint() const override { return QSize(-1,-1); }
     QSize minimumSizeHint() const override { return QSize(-1,-1); }
};

#endif
