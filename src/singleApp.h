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

#ifndef SINGLEAPP_H
#define SINGLEAPP_H

#include <QApplication>
#include <QLockFile>
#include <QLocalServer>
#include <QLocalSocket>

class singleApp : public QApplication
{
    Q_OBJECT

private:
    QLockFile *m_lockFile;
    QLocalServer *m_fifoFile;

    const QString message;

private:
    singleApp();

    void acceptMessage();

signals:
    void sendMessage(QString msg);

public:
    singleApp(int & argc, char ** argv);
    ~singleApp() override;

    bool isRunning();
};

#endif
