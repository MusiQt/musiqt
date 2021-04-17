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

#include "loader.h"
#include "input/inputFactory.h"

#include <QDebug>

player::player() :
    m_input(IFACTORY->get()),
    m_preload(IFACTORY->get()),
    m_audio(new audio)
{
    connect(m_audio.data(), &audio::updateTime,  this, &player::updateTime);
    connect(m_audio.data(), &audio::songEnded,   this, &player::songEnded);
    connect(m_audio.data(), &audio::preloadSong, this, &player::preloadSong);
}

/*
 * Starts or resumes playback.
 * If already playing, this has no effect.
 * If paused, playback resumes from the current position.
 */
void player::play()
{
    if (m_audio->play(m_input.data()))
        emit stateChanged();
}

/*
 * Stops playback.
 * If playback is already stopped, this has no effect.
 */
void player::stop()
{
    if (m_audio->stop())
        emit stateChanged();
}

/*
 * Pauses playback.
 * If playback is already paused, this has no effect.
 */
void player::pause()
{
    m_audio->pause();
    emit stateChanged();
}

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

/*
 * Pauses playback.
 * If playback is already paused, resumes playback.
 * If playback is stopped, starts playback.
 */
// void player::playpause() TODO

bool player::tryPreload(const QString& song)
{
    QString songPreloaded = m_preload->songLoaded();
    if (songPreloaded.isEmpty())
        return false;

    input* i = m_preload.take();
    m_preload.reset(IFACTORY->get());

    if (!songPreloaded.compare(song))
    {
        m_input.reset(i);
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

void player::loadAndPlay(const QString& filename)
{
    // Start playing once loaded
    QMetaObject::Connection * const connection = new QMetaObject::Connection;
    *connection = connect(this, &player::songLoaded,
        [this, connection] ()
        {
            play();

            QObject::disconnect(*connection);
            delete connection;
        });

    load(filename);
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
