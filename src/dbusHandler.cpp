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

#include "dbusHandler.h"

#include "player.h"

#include "mediaplayer2adaptor.h"
#include "playeradaptor.h"

#include <QDate>

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

// TODO move into backends
static QStringList mimeTypes = QStringList()
#if defined HAVE_MPG123 || defined HAVE_LIBAVFORMAT_AVFORMAT_H
    << "audio/mpeg"
    << "audio/mpeg3"
    << "audio/x-mpeg-3"
#endif
#if defined HAVE_VORBIS || defined HAVE_LIBAVFORMAT_AVFORMAT_H
    << "audio/ogg"
    << "audio/vorbis"
#endif
#if defined HAVE_OPUS || defined HAVE_LIBAVFORMAT_AVFORMAT_H
    << "audio/opus"
#endif
#ifdef HAVE_OPENMPT
    << "audio/it"
    << "audio/s3m"
    << "audio/xm"
#endif
#ifdef HAVE_SIDPLAYFP
    << "audio/prs.sid"
    << "audio/x-psid"
#endif
#ifdef HAVE_WAVPACK
    << "audio/x-wv"
#endif
#ifdef HAVE_LIBMPCDEC
    << "audio/musepack"
    << "audio/x-musepack"
#endif
#if defined HAVE_SNDFILE || defined HAVE_LIBAVFORMAT_AVFORMAT_H
    << "audio/wav"
    << "audio/aiff"
    << "audio/x-aiff"
    << "audio/x-au"
    << "audio/x-wav"
#endif
#ifdef HAVE_LIBAVFORMAT_AVFORMAT_H
    << "audio/x-realaudio"
    << "audio/x-pn-realaudio"
#endif
;

dbusHandler::dbusHandler(player* p, QObject* parent) :
    QObject(parent),
    m_player(p)
{
    new MediaPlayer2Adaptor(this);
    new PlayerAdaptor(this);

    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerService("org.mpris.MediaPlayer2.musiqt");
    dbus.registerObject("/org/mpris/MediaPlayer2", this);
    dbus.registerObject("/org/mpris/MediaPlayer2/Player", this);
}

// MediaPlayer2 properties
bool dbusHandler::canQuit() const { return true; }
bool dbusHandler::canRaise() const { return true; }
bool dbusHandler::hasTrackList() const { return false; }
QString dbusHandler::identity() const { return QApplication::applicationName(); }
QStringList dbusHandler::supportedUriSchemes() const { return QStringList("file"); }
QStringList dbusHandler::supportedMimeTypes() const { return mimeTypes; }

// MediaPlayer2 methods
void dbusHandler::Raise()
{
    const QWidgetList topLevelWidgets = QApplication::topLevelWidgets();
    for (QWidget *widget : topLevelWidgets)
    {
        if (widget->isWindow())
        {
            widget->setWindowState(Qt::WindowActive);
            widget->raise();
        }
    }
}

void dbusHandler::Quit() { QApplication::instance()->quit(); }

// MediaPlayer2.Player properties
bool dbusHandler::canGoNext() const { return false; }
bool dbusHandler::canGoPrevious() const { return false; }
bool dbusHandler::canPlay() const { return true; }
bool dbusHandler::canPause() const { return true; }
bool dbusHandler::canSeek() const { return m_player->seekable(); }
bool dbusHandler::canControl() const { return true; }

QString dbusHandler::loopStatus() const { return QString("None"); } // "None", "Track" or "Playlist"
void dbusHandler::setLoopStatus(const QString &value) { /* ignore */ }

double dbusHandler::maximumRate() const { return 1.; }
double dbusHandler::minimumRate() const { return 1.; }

QString getDate(QString year)
{
    QDate date(year.toInt(), 1, 1);
    return date.toString(Qt::ISODate);
}

QVariantMap dbusHandler::metadata() const
{
    const metaData* data = m_player->getMetaData();

    QVariantMap mprisData;
    //mprisData.insert("mpris:trackid", QDBusObjectPath(...));
    mprisData.insert("mpris:length", qlonglong(m_player->songDuration())*1000ll);
    mprisData.insert("xesam:title", data->getInfo(metaData::TITLE));
    mprisData.insert("xesam:artist", data->getInfo(metaData::ARTIST));
    mprisData.insert("xesam:album", data->getInfo(metaData::ALBUM));
    mprisData.insert("xesam:trackNumber", data->getInfo(metaData::TRACK_NUMBER).toInt());
    mprisData.insert("xesam:genre", QStringList(data->getInfo(metaData::GENRE)));
    mprisData.insert("xesam:comment", QStringList(data->getInfo(metaData::COMMENT)));
    mprisData.insert("xesam:asText", data->getInfo(metaData::AS_TEXT));
    mprisData.insert("xesam:contentCreated", getDate(data->getInfo(metaData::CONTENT_CREATED)));
    
    mprisData.insert("xesam:url", data->getInfo(metaData::URL));

    return mprisData;
}

QString dbusHandler::playbackStatus() const
{
    switch (m_player->state())
    {
    case state_t::PLAY:  return QString("Playing");
    case state_t::PAUSE: return QString("Paused");
    case state_t::STOP:  return QString("Stopped");
    default: return QString();
    }
}

qlonglong dbusHandler::position() const
{
    // (position / 100) * duration * 1000
    return m_player->getPosition() * m_player->songDuration() * 10ll;
}
    
double dbusHandler::rate() const { return 1.; }
void dbusHandler::setRate(double value) { /* ignore */ }
double dbusHandler::volume() const { return m_player->getVolume() / 100.; }
void dbusHandler::setVolume(double value) { m_player->setVolume(value*100.); }

// MediaPlayer2.Player methods
void dbusHandler::Play() { m_player->play(); }
void dbusHandler::Pause() { m_player->pause(); }

void dbusHandler::PlayPause()
{
    switch (m_player->state())
    {
    default:
    case state_t::PLAY: // fall-through
    case state_t::PAUSE: m_player->pause(); break;
    case state_t::STOP:  m_player->play();  break;
    }
}

void dbusHandler::Stop() { m_player->stop(); }
void dbusHandler::Previous() { /* ignore */ }
void dbusHandler::Next() { /* ignore */ }

void dbusHandler::Seek(qlonglong Offset)
{
    qlonglong Position = position() + Offset;
    m_player->setPosition((Position/1000)/m_player->songDuration());
}

void dbusHandler::SetPosition(const QDBusObjectPath &TrackId, qlonglong Position)
{
    // TODO check TrackId
    m_player->setPosition((Position/1000)/m_player->songDuration());
}

void dbusHandler::OpenUri(const QString &Uri) { /* TODO */ }
