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

#include "player.h"

#include "audio.h"
#include "inputFactory.h"
#include "loader.h"
#include "input/input.h"

#include <QDebug>

player::player() :
    m_input(IFACTORY->get()),
    m_audio(new audio),
    m_preload(IFACTORY->get())
{
    connect(m_audio.data(), &audio::updateTime,  this, &player::updateTime);
    connect(m_audio.data(), &audio::songEnded,   this, &player::songEnded);
    connect(m_audio.data(), &audio::preloadSong, this, &player::preloadSong);
    connect(m_audio.data(), &audio::audioError,  this, &player::onError);
}

player::~player() = default;

void player::onError(const QString& error)
{
    stop();
    emit audioError(error);
}

void player::play()
{
    try
    {
        if (m_audio->play(m_input.data()))
            emit stateChanged();
    }
    catch (audio::initError const &e)
    {
        emit audioError(e.message());
    }
}

void player::stop()
{
    if (m_audio->stop())
        emit stateChanged();
}

void player::pause()
{
    m_audio->pause();
    emit stateChanged();
}

state_t player::state() const { return m_audio->state(); }

int player::seconds() const { return m_audio->getPosition()/1000; }

void player::setPosition(double pos)
{
    m_audio->seek(pos);
    emit positionChanged();
}

int player::getPosition() const
{
    unsigned int duration = m_input->songDuration();
    return duration ? (100*m_audio->getPosition())/duration : 0;
}

void player::setVolume(int vol)
{
    m_audio->setVolume(vol);
    emit volumeChanged();
}

int player::getVolume() const { return m_audio->getVolume(); }

bool player::tryPreload(const QString& song)
{
    QString songPreloaded = m_preload->songLoaded();
    if (songPreloaded.isEmpty())
        return false;

    input* i = m_preload.release();
    m_preload.reset(IFACTORY->get());

    if (!songPreloaded.compare(song))
    {
        m_input.reset(i);
        emit songChanged();
        return true;
    }
    else
    {
        return false;
    }
}

void player::load(const QString& filename)
{
    loader* fileLoader = new loader(filename);
    connect(fileLoader, &loader::loaded, this, &player::loaded);
    connect(fileLoader, &loader::finished, fileLoader, &loader::deleteLater);
    fileLoader->start();
}

void player::loaded(input* res)
{
    state_t state = m_audio->state();
    stop();

    bool loaded;
    if (res != nullptr)
    {
        m_input.reset(res);

        if (state == state_t::PLAY)
            play();

        loaded = true;
    }
    else
    {
        m_input.reset(IFACTORY->get());

        loaded = false;
    }

    emit songLoaded(loaded);
}

void player::preload(const QString& filename)
{
    loader* fileLoader = new loader(filename);
    connect(fileLoader, &loader::loaded, this, &player::preloaded);
    connect(fileLoader, &loader::finished, fileLoader, &loader::deleteLater);
    fileLoader->start();
}

const metaData* player::getMetaData() const { return m_input->getMetaData(); }

bool player::seekable() const { return m_input->seekable(); }

bool player::gapless() const { return m_input->gapless(); }

unsigned int player::subtunes() const { return m_input->subtunes(); }

unsigned int player::subtune() const { return m_input->subtune(); }

QString player::loadedSong() const { return m_input->songLoaded(); }

unsigned int player::songDuration() const { return m_input->songDuration(); }

void player::preloaded(input* res)
{
    if ((res != nullptr) && m_audio->gapless(res))
    {
        m_preload.reset(res);

        qDebug() << "Song preloaded";
    }
    else
    {
        m_preload.reset(IFACTORY->get());

        qDebug() << "Discard preloaded song";
    }
}

void player::playOnLoad()
{
    // Start playing once a song is loaded
#if QT_VERSION >= 0x060000 
    connect(this, &player::songLoaded, this, [=] (bool res) { if (res) play(); }, Qt::SingleShotConnection);
#else
    QObject* ctx = new QObject();
    connect(this, &player::songLoaded, ctx, [=] (bool res) { if (res) play(); ctx->deleteLater(); });
#endif
}

void player::changeSubtune(dir_t dir)
{
    if (m_input->subtunes() <= 1)
        return;

    unsigned int i = m_input->subtune();
    switch (dir)
    {
    case dir_t::ID_PREV:
        i--;
        break;
    case dir_t::ID_NEXT:
        i++;
        break;
    }

    if ((i < 1) || (i > m_input->subtunes()))
        return;

    if (m_input->subtune(i))
        emit subtunechanged();
}
