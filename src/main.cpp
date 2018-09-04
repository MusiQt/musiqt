/*
 *  Copyright (C) 2013-2018 Leandro Nini
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
#include "translator.h"

#include <QLocale>
#include <QTime>
#include <QSplashScreen>

#ifdef HAVE_CONFIG_H
#  include "config.h"
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


#if QT_VERSION >= 0x050000
void messageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
#else
void messageOutput(QtMsgType type, const char *msg)
#endif
{
    QByteArray timeMsg = QTime::currentTime().toString("hh:mm:ss.zzz").toLocal8Bit();
#if QT_VERSION >= 0x050000
    QByteArray localMsg = msg.toLocal8Bit();
#else
    QByteArray localMsg = msg;
#endif
    switch (type)
    {
    case QtDebugMsg:
        fprintf(stderr, "%s:D: %s\n", timeMsg.constData(), localMsg.constData());
        break;
#if QT_VERSION >= 0x050000
    case QtInfoMsg:
        fprintf(stderr, "%s:I: %s\n", timeMsg.constData(), localMsg.constData());
        break;
#endif
    case QtWarningMsg:
        fprintf(stderr, "%s:W: %s\n", timeMsg.constData(), localMsg.constData());
        break;
    case QtCriticalMsg:
        fprintf(stderr, "%s:C: %s\n", timeMsg.constData(), localMsg.constData());
        break;
    case QtFatalMsg:
        fprintf(stderr, "%s:F: %s\n", timeMsg.constData(), localMsg.constData());
        break;
    }
    fflush(stderr);
}

int main(int argc, char *argv[])
{
    singleApp app(argc, argv);
    if (app.isRunning())
        return -1;

#if QT_VERSION >= 0x050000
    qInstallMessageHandler(messageOutput);
#else
    qInstallMsgHandler(messageOutput);
#endif

    QPixmap pixmap(":/resources/splash.png");
    QSplashScreen splash(pixmap, Qt::WindowStaysOnTopHint);
    splash.show();
    splash.clearMessage(); // Splash doesn't show on Linux without this (???)
    app.processEvents();

    app.setOrganizationName("DrFiemost");
    app.setApplicationName(PACKAGE_NAME);

#ifdef ENABLE_NLS
    // Add translator
    app.installTranslator(new translator(&app));
#endif

    mainWindow window;
    QObject::connect(&app, SIGNAL(sendMessage(QString)), &window, SLOT(onMessage(QString)));

    window.show();
    app.processEvents();

    window.init(argc>1 ? argv[1] : nullptr);
    app.processEvents();

    splash.finish(&window);

    return app.exec();
}
