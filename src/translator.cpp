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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "translator.h"

#include "gettext.h"
#include <locale.h>

#include <QDebug>
#include <QCoreApplication>

#ifdef _WIN32
#  include <windows.h>
#endif

translator::translator(QObject* parent) :
    QTranslator(parent) 
{
    char* locale = setlocale(LC_ALL, "");
    qDebug() << "locale: " << locale;
#ifndef _WIN32
    bindtextdomain(PACKAGE, LOCALEDIR);
#else
    QString cmd = QString(QCoreApplication::applicationDirPath()).append("/locale");
    qDebug() << "localedir: " << cmd;
    bindtextdomain(PACKAGE, cmd.toLocal8Bit().constData());
#endif

    textdomain(PACKAGE);
}

translator::~translator() {}

#if QT_VERSION >= 0x050000
QString translator::translate(const char* context, const char* sourceText, const char* disambiguation, int n) const
#else
QString translator::translate(const char* context, const char* sourceText, const char* disambiguation) const
#endif
{
    return QString::fromUtf8(gettext(sourceText));
}
