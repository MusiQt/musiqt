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

#ifndef LASTFM_H
#define LASTFM_H

#include <libs/liblastfm/Audioscrobbler.h>

#include <QNetworkReply>
#include <QObject>
#include <QPointer>
#include <QScopedPointer>
#include <QString>
#include <QTimer>

class player;
class metaData;

/*****************************************************************/

class lastfmScrobbler : public QObject
{
    Q_OBJECT

public:
    lastfmScrobbler(player* p, QObject* parent = nullptr);
    ~lastfmScrobbler();

    void setScrobbling(bool scrobble);

signals:
    void notify(const QString &title, const QString &text);

private:
    void stateChanged();
    void songChanged();
    void scrobble();

    void onScrobblesCached(const QList<lastfm::Track>& tracks);
    void onScrobblesSubmitted(const QList<lastfm::Track>& tracks);
    void onNowPlayingReturn();

    void nowPlaying(bool force=false);

private:
    QPointer<lastfm::Audioscrobbler> m_scrobbler;
    QScopedPointer<lastfm::Track> m_track;
    QPointer<QNetworkReply> m_nowPlayingReply;
    player *m_player;
    QTimer m_timer;
    bool m_enable;
};

/*****************************************************************/

#include "configFrame.h"

class lastfmConfig : public configFrame
{
    Q_OBJECT

private:
    lastfmConfig() {}
    lastfmConfig(const lastfmConfig&);
    lastfmConfig& operator=(const lastfmConfig&);

private:
    void auth();
    void gotToken();
    void setSession(const QString &userName, const QString &sessionKey);

signals:
    void usernameChanged(const QString &text);
    void sessionChanged(const QString &text);

public:
    lastfmConfig(QWidget* win);
    virtual ~lastfmConfig() {}
};

#endif
