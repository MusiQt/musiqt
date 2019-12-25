/*
 *  Copyright (C) 2010-2019 Leandro Nini
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

#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include <QTranslator>

class translator : public QTranslator
{
    Q_OBJECT

private:
    translator() {}

public:
    translator(QObject* parent=nullptr);
    virtual ~translator();

#if QT_VERSION >= 0x050000
    QString translate(const char* context, const char* sourceText, const char* disambiguation=Q_NULLPTR, int n=-1) const override;
#else
    QString translate(const char* context, const char* sourceText, const char* disambiguation=0) const override;
#endif
};

#endif
