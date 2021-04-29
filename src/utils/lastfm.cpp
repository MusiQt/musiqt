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

#include "lastfm.h"

#include "player.h"
#include "input/metaData.h"
#include "utils/xdg.h"

#include <lastfm5/misc.h>
#include <lastfm5/ws.h>
#include <lastfm5/Track.h>
#include <lastfm5/XmlQuery.h>

#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QDebug>

QString signature(QString method)
{
    QString sig = QString("api_key%1method%2token%3").arg(lastfm::ws::ApiKey, method, lastfm::ws::SharedSecret);
    return lastfm::md5(sig.toUtf8());
}

/*****************************************************************/

lastfmScrobbler::lastfmScrobbler(player* p, QObject* parent) :
    QObject(parent),
    m_scrobbler("mqt"),
    m_player(p)
{
    QSettings settings;
    QString sessionKey = settings.value("Last.fm Settings/Session Key", QString()).toString();

    lastfm::ws::ApiKey          = "5cfc6d1c438c0bb0d9f2e1d6d2abd9fd";
    lastfm::ws::SharedSecret    = "5785cfb120eee3142c18bd3901494d8b";
    lastfm::ws::SessionKey      = sessionKey;

    connect(m_player, &player::stateChanged, this, &lastfmScrobbler::stateChanged);
}

void lastfmScrobbler::stateChanged()
{
    if (m_player->state() == state_t::PLAY)
    {
        nowPlaying(m_player->getMetaData());
    }
}

void lastfmScrobbler::nowPlaying(const metaData* data)
{
    lastfm::MutableTrack track;
    track.setArtist(data->getInfo(metaData::ARTIST));
    track.setAlbum(data->getInfo(metaData::ALBUM));
    track.setTitle(data->getInfo(metaData::TITLE));
    track.stamp(); //sets track start time

    m_scrobbler.nowPlaying(track);
}

/*****************************************************************/

lastfmConfig::lastfmConfig(QWidget* win) :
    configFrame(win)
{
    QPushButton* button = new QPushButton(tr("Authenticate"), this);
    button->setToolTip("Get session key from Last.fm");
    matrix()->addWidget(button);
    connect(button, &QPushButton::clicked, this, &lastfmConfig::auth);
}

void lastfmConfig::auth()
{
    // Fetch a request token
    QMap<QString, QString> params;
    params["method"] = "auth.getToken";
    params["api_key"] = lastfm::ws::ApiKey;
    params["api_sig "] = signature("auth.getToken");

    QNetworkReply* reply = lastfm::ws::post(params);
    QObject::connect(reply, &QNetworkReply::finished, this, &lastfmConfig::gotToken);
}

void lastfmConfig::gotToken()
{
    QNetworkReply* reply = static_cast<QNetworkReply*>(sender());
    QString token;

    lastfm::XmlQuery query;
    if (!query.parse(reply))
    {
        qWarning() << query.parseError().message();
        return;
    }
    token = query["token"].text();
    qDebug() << token;

    // Request authorization from the user
    QString link = QString("http://www.last.fm/api/auth/?api_key=%1&token=%2").arg(lastfm::ws::ApiKey, token);
    xdg::open(link);

    // Wait before asking for session key
    QMessageBox::information(this, tr("Info"), tr("Authorize app on Last.fm site before proceeding"));

    // Fetch A Web Service Session
    QMap<QString, QString> params;
    params["method"] = "auth.getSession";
    params["api_key"] = lastfm::ws::ApiKey;
    params["token"] = token;
    params["api_sig "] = signature("auth.getSession");

    reply = lastfm::ws::post(params);
    QObject::connect(reply, &QNetworkReply::finished,
        [reply]()
        {
            lastfm::XmlQuery query;
            if (query.parse(reply))
            {
                QString sessionKey = query["session"]["key"].text();
                //lastfm::ws::Username = query["session"]["name"].text();
                lastfm::ws::SessionKey = sessionKey;
                qDebug() << sessionKey;

                QSettings settings;
                settings.setValue("Last.fm Settings/Session Key", sessionKey);
            }
            else
            {
                qWarning() << query.parseError().message();
            }
        }
    );
}
