/*
 *  Copyright (C) 2013-2023 Leandro Nini
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

#include "mainWindow.h"
#include "player.h"
#include "xdg.h"

#include <QSplashScreen>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMutex>

#include <iostream>

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#ifdef ENABLE_NLS
#  include "translator.h"
#endif

#ifdef ENABLE_DBUS
#  include "dbusHandler.h"
#endif

#ifdef HAVE_LASTFM
#  include "lastfm.h"
#endif

#ifdef QT_STATICPLUGIN
#include <QtPlugin>
#  ifdef _WIN32
    Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin)
    Q_IMPORT_PLUGIN(QWindowsAudioPlugin)
    Q_IMPORT_PLUGIN(QWindowsVistaStylePlugin)
#  endif
Q_IMPORT_PLUGIN(QGifPlugin)
Q_IMPORT_PLUGIN(QJpegPlugin)
Q_IMPORT_PLUGIN(QTiffPlugin)
#endif

QMutex logMutex;
QFile logFile;

void messageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QString message = qFormatLogMessage(type, context, msg);

    if (logFile.isOpen()) {
        QMutexLocker lock(&logMutex);
        logFile.write(message.toLocal8Bit().append('\n'));
        logFile.flush();
    } else {
        std::cerr << message.toLocal8Bit().constData() << std::endl;
    }
}

int main(int argc, char *argv[])
{
    singleApp app(argc, argv);

    app.setOrganizationName("DrFiemost");
    app.setApplicationName(PACKAGE_NAME);
    app.setApplicationVersion(PACKAGE_VERSION);

    // Check if another instance is already running
    if (app.isRunning())
        return -1;

    // Init log
    QString stateDir = xdg::getStateDir();
    stateDir.append('/').append(app.organizationName());
    QDir().mkpath(stateDir);

    const QString logFileName(QString("%1/musiqt.log").arg(stateDir));
    logFile.setFileName(logFileName);
    if (!logFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        std::cerr << "Cannot open log file, logging to stderr" << std::endl;
    } else {
        QFileInfo fInfo(logFile);
        std::cout << "Logging to " << fInfo.absoluteFilePath().toLocal8Bit().constData() << std::endl;
    }

    qSetMessagePattern("%{time hh:mm:ss.zzz}:"
        "%{if-debug}D%{endif}%{if-info}I%{endif}%{if-warning}W%{endif}%{if-critical}C%{endif}%{if-fatal}F%{endif}: "
        "%{message}");

    qInstallMessageHandler(messageOutput);

    // Show splash screen
    QPixmap pixmap(":/resources/splash.png");
    QSplashScreen splash(pixmap, Qt::WindowStaysOnTopHint);
    splash.show();
    splash.clearMessage(); // Splash doesn't show on Linux without this (???)
    app.processEvents();

#ifdef ENABLE_NLS
    // Add translator
    app.installTranslator(new translator(&app));
#endif

    player p;

#ifdef ENABLE_DBUS
    dbusHandler dbus(&p);
#endif
#ifdef HAVE_LASTFM
    lastfmScrobbler scrobbler(&p);
#endif

    // Create GUI
    mainWindow window(&p);
    QObject::connect(&app, &singleApp::sendMessage, &window, &mainWindow::onMessage);
#ifdef HAVE_LASTFM
    QObject::connect(&window, &mainWindow::setScrobbling, &scrobbler, &lastfmScrobbler::setScrobbling);
    QObject::connect(&scrobbler, &lastfmScrobbler::notify, &window, &mainWindow::notify);
#endif

    window.show();
    app.processEvents();

    window.init(argc>1 ? argv[1] : nullptr);
    app.processEvents();

    splash.finish(&window);

    return app.exec();
}
