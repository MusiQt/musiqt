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

#include "singleApp.h"

#include "mainWindow.h"
#include "utils.h"
#include "xdg.h"

#ifdef _WIN32
#  include <windows.h>
#endif

#include <QDir>
#include <QDebug>

singleApp::singleApp(int & argc, char ** argv) :
    QApplication(argc, argv),
    _lockFile(nullptr),
    _fifoFile(nullptr),
    message(argc > 1 ? argv[1] : "")
{}

bool singleApp::isRunning()
{
#ifdef _WIN32
    const QString fifoFileName("\\\\.\\pipe\\musiqt.fifo");
#else
    const QString fifoFileName(QString("%1/.musiqt.fifo").arg(xdg::getRuntimeDir()));
#endif

    const QString runtimeDir = xdg::getRuntimeDir();
    QDir().mkpath(runtimeDir);

    const QString lockFileName(QString("%1/.musiqt.lock").arg(runtimeDir));
    qDebug() << lockFileName;

    _lockFile = new QLockFile(lockFileName);
    if (_lockFile->tryLock())
    {
        _lockFile->setStaleLockTime(0);

        // delete stale fifo if any
        QFile::remove(fifoFileName);

        _fifoFile = new QLocalServer(this);
        if (_fifoFile->listen(fifoFileName))
        {
            connect(_fifoFile, &QLocalServer::newConnection, this, &singleApp::acceptMessage);
        }
        else
        {
            qWarning() << _fifoFile->errorString();
            utils::delPtr(_fifoFile);
        }
        return false;
    }
    else
    {
        utils::delPtr(_lockFile);
        qWarning("Already running");
        if (!message.isEmpty())
        {
            QLocalSocket socket(this);
            qDebug() << fifoFileName;
            socket.connectToServer(fifoFileName);
            if (socket.waitForConnected())
            {
                const QString path = QDir(message).absolutePath();
                qDebug() << path;
                QByteArray block;
                QDataStream out(&block, QIODevice::WriteOnly);
                out.setVersion(QDataStream::Qt_4_0);
                out << path;
                qint64 res = socket.write(block);
                qDebug() << "res: " << res;
                socket.flush();
                socket.disconnectFromServer();
            }
            else
            {
                qWarning() << socket.errorString();
            }
            socket.close();
        }
        return true;
    }
}

singleApp::~singleApp()
{
    if (_lockFile != nullptr)
    {
        delete _lockFile;
    }

    if (_fifoFile != nullptr)
    {
        QFile::remove(_fifoFile->serverName());
        delete _fifoFile;
    }
}

void singleApp::acceptMessage()
{
    QLocalSocket* socket = _fifoFile->nextPendingConnection();
    if (socket == nullptr)
    {
        qWarning() << _fifoFile->errorString();
        return;
    }

    socket->waitForReadyRead();

    qDebug() << "data: " << socket->bytesAvailable();
    QDataStream in(socket);
    in.setVersion(QDataStream::Qt_4_0);
    QString message;
    in >> message;
    qDebug() << "msg: " << message;

    if (!message.isEmpty())
        emit sendMessage(message);

    delete socket;
}
