/*
 *  Copyright (C) 2010-2023 Leandro Nini
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

#include <QDir>
#include <QDebug>

singleApp::singleApp(int & argc, char ** argv) :
    QApplication(argc, argv),
    m_lockFile(nullptr),
    m_fifoFile(nullptr),
    message(argc > 1 ? argv[1] : "")
{}

bool singleApp::isRunning()
{
    const QString runtimeDir = xdg::getRuntimeDir();
    QDir().mkpath(runtimeDir);

#ifdef _WIN32
    const QString fifoFileName(R"(\\.\pipe\musiqt.fifo)");
#else
    const QString fifoFileName(QString("%1/.musiqt.fifo").arg(runtimeDir));
#endif
    qDebug() << "FIFO file: " << fifoFileName;

    const QString lockFileName(QString("%1/.musiqt.lock").arg(runtimeDir));
    qDebug() << "Lock file: " << lockFileName;

    m_lockFile = new QLockFile(lockFileName);
    if (m_lockFile->tryLock())
    {
        m_lockFile->setStaleLockTime(0);

        // delete stale fifo if any
        QFile::remove(fifoFileName);

        m_fifoFile = new QLocalServer(this);
        if (m_fifoFile->listen(fifoFileName))
        {
            connect(m_fifoFile, &QLocalServer::newConnection, this, &singleApp::acceptMessage);
        }
        else
        {
            qWarning() << m_fifoFile->errorString();
            utils::delPtr(m_fifoFile);
        }
        return false;
    }
    else
    {
        utils::delPtr(m_lockFile);
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
                socket.waitForBytesWritten();
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
    if (m_lockFile != nullptr)
    {
        delete m_lockFile;
    }

    if (m_fifoFile != nullptr)
    {
        QFile::remove(m_fifoFile->serverName());
        delete m_fifoFile;
    }
}

void singleApp::acceptMessage()
{
    QLocalSocket* socket = m_fifoFile->nextPendingConnection();
    if (socket == nullptr)
    {
        qWarning() << m_fifoFile->errorString();
        return;
    }

    if (socket->waitForReadyRead())
    {
        qDebug() << "data: " << socket->bytesAvailable();
        QDataStream in(socket);
        in.setVersion(QDataStream::Qt_4_0);
        QString message;
        in >> message;
        qDebug() << "msg: " << message;

        if (!message.isEmpty())
            emit sendMessage(message);
    }

    delete socket;
}
