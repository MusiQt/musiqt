/*
 *  Copyright (C) 2013-2017 Leandro Nini
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

#include "singleApp.h"

#include <QIcon>

#include "mainWindow.h"
#include "translator.h"

#ifdef QT_STATICPLUGIN
#include <QtPlugin>
#  ifdef _WIN32
    Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin);
    Q_IMPORT_PLUGIN(QWindowsAudioPlugin);
#  endif
Q_IMPORT_PLUGIN(QGifPlugin)
Q_IMPORT_PLUGIN(QJpegPlugin)
Q_IMPORT_PLUGIN(QTiffPlugin)
#endif

int main(int argc, char *argv[])
{
    singleApp app(argc, argv);
    if (app.isRunning())
        return -1;

    app.setOrganizationName("DrFiemost");
    app.setApplicationName("musiqt");

#ifdef ENABLE_NLS
    // Add translator
    app.installTranslator(new translator(&app));
#endif

    mainWindow window;
    window.init(argc>1 ? argv[1] : nullptr);

    window.show();

    return app.exec();
}
