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

#include "singleApp.h"

#include "mainWindow.h"
#include "utils.h"
#include "xdg.h"

#ifdef _WIN32
#  include <windows.h>
#else
#  include <fcntl.h>
#  include <X11/XF86keysym.h>
#endif

#include <QDir>
#include <QDebug>

singleApp::singleApp(int & argc, char ** argv) :
    QApplication(argc, argv),
    _lockFile(nullptr),
    _fifoFile(nullptr),
    _lockFileName(QString("%1/.musiqt.lock").arg(xdg::getRuntimeDir())),
#ifdef _WIN32
    _fifoFileName("\\\\.\\pipe\\musiqt.fifo")
#else
    _fifoFileName(QString("%1/.musiqt.fifo").arg(xdg::getRuntimeDir()))
#endif
{
    _lockFile = new QFileEx(_lockFileName);
    if (_lockFile->open(QIODevice::ReadWrite)
        && !_lockFile->lock(0, 1))
    {
        _lockFile->close();
        utils::delPtr(_lockFile);
        qWarning("Already running");
        if (argc > 1)
        {
            QLocalSocket socket(this);
            qDebug() << _fifoFileName;
            socket.connectToServer(_fifoFileName);
            if (socket.waitForConnected())
            {
                const QString message = QDir(QString(argv[1])).absolutePath();
                qDebug() << message;
                QByteArray block;
                QDataStream out(&block, QIODevice::WriteOnly);
                out.setVersion(QDataStream::Qt_4_0);
                out << message;
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
        running = true;
    }
    else
    {
        // delete stale fifo
        QFile::remove(_fifoFileName);

        _fifoFile = new QLocalServer(this);
        if (_fifoFile->listen(_fifoFileName))
        {
            connect(_fifoFile, SIGNAL(newConnection()), this, SLOT(acceptMessage()));
        }
        else
        {
            qWarning() << _fifoFile->errorString();
            utils::delPtr(_fifoFile);
        }
        running = false;
    }
}

singleApp::~singleApp()
{
    if (_lockFile != nullptr)
    {
        if (_lockFile->isOpen())
            _lockFile->close();
        delete _lockFile;
        QFile::remove(_lockFileName);
    }

    if (_fifoFile != nullptr)
    {
        delete _fifoFile;
        QFile::remove(_fifoFileName);
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
