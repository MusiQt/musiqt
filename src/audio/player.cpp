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

#include "input/inputFactory.h"

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

void player::loaded(input* res, bool subtunes)
{
    state_t state = m_audio->state();
    stop();

    if (res != nullptr)
    {
        m_input.reset(res);

        if (subtunes)
            m_input->subtune(1);

        if (state == state_t::PLAY)
            play();
    }
    else
    {
        m_input.reset(IFACTORY->get());
    }
}

void player::preloaded(input* res, bool subtunes)
{
    if ((res != nullptr) && m_audio->gapless(res))
    {
        m_preload.reset(res);

        if (subtunes)
            m_preload->subtune(1);
    }
    else
    {
        m_preload.reset(IFACTORY->get());
    }
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