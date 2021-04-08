/*
 *  Copyright (C) 2021 Leandro Nini
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

#ifndef DBUS_HANDLER_H
#define DBUS_HANDLER_H

#include <QApplication>
#include <QObject>
#include <QtDBus>
#include <QWidget>

#include <QDebug>

class dbusHandler : public QObject
{
    Q_OBJECT

public:
    dbusHandler(QObject* parent = nullptr);

    Q_PROPERTY(bool CanQuit READ getCanQuit)
    Q_PROPERTY(bool CanRaise READ getCanRaise)
    Q_PROPERTY(bool HasTrackList READ getHasTrackList)
    Q_PROPERTY(QString Identity READ getIdentity)
    Q_PROPERTY(QStringList SupportedUriSchemes READ getSupportedUriSchemes)
    Q_PROPERTY(QStringList SupportedMimeTypes READ getSupportedMimeTypes)

    bool getCanQuit() const;
    bool getCanRaise() const;
    bool getHasTrackList() const;
    QString getIdentity() const;
    QStringList getSupportedUriSchemes() const;
    QStringList getSupportedMimeTypes() const;

public slots:
    void Raise();
    void Quit();

    void Play();
    void Pause();
    void PlayPause();
    void Stop();
    void Previous();
    void Next();
    void Seek(qlonglong Offset);
    void SetPosition(const QDBusObjectPath &TrackId, qlonglong Position);
    void OpenUri(const QString &Uri);
};

#endif
