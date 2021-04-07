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

player::player() :
    m_input(IFACTORY->get()),
    m_preload(IFACTORY->get()),
    m_audio(new audio)
{
    connect(m_audio.data(), &audio::updateTime,  this, &player::updateTime);
    connect(m_audio.data(), &audio::songEnded,   this, &player::songEnded);
    connect(m_audio.data(), &audio::preloadSong, this, &player::preloadSong);
}

bool player::tryPreload(const QString& song)
{
    QString songPreloaded = m_preload->songLoaded();
    if (!songPreloaded.isEmpty())
    {
        if (songPreloaded.compare(song))
        {
            m_preload.reset(IFACTORY->get());
            return false;
        }
        else
        {
            m_input.reset(m_preload.take());
            m_preload.reset(IFACTORY->get());
            return true;
        }
    }
            return false;
}

void player::loaded(input* res, bool subtunes)
{
    m_input.reset(res);

    if (subtunes)
        m_input->subtune(1);
}

void player::preloaded(input* res, bool subtunes)
{
    m_preload.reset(res);
    if ((res != nullptr) && m_audio->gapless(res))
    {
        if (subtunes)
            m_preload->subtune(1);

        //qDebug() << "Song preloaded";
    }
    else
    {
        m_preload.reset(IFACTORY->get());
    }
}
