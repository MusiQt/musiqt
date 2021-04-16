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

#ifndef PLAYER_H
#define PLAYER_H

#include "audio.h"
#include "input/input.h"

#include <QScopedPointer>

enum class dir_t
{
    ID_PREV,
    ID_NEXT
};

/*****************************************************************/

class player : public QObject
{
    Q_OBJECT

private:
    QScopedPointer<input> m_input;
    QScopedPointer<input> m_preload;
    QScopedPointer<audio> m_audio;

private:
    player(const player&);
    player& operator=(const player&);

    void loaded(input* res);

    void preloaded(input* res);

signals:
    void stateChanged();

    void subtunechanged();

    void updateTime();
    void songEnded();
    void preloadSong();

    void songLoaded(bool loaded);
    void volumeChanged();
    void positionChanged();

public:
    player();
    ~player() {}

    /// Start playback
    void play();

    /// Pause playback
    void pause();

    /// Stop playback
    void stop();

    /// Get playback state
    state_t state() const { return m_audio->state(); }

    /// Get current position in seconds
    int seconds() const { return m_audio->getPosition()/1000; }

    /// Set current position [0,1]
    void setPosition(double pos);

    /// Get current position in percent
    int getPosition() const;

    /// Set volume
    void setVolume(int vol);

    /// Get volume
    int getVolume() const { return m_audio->getVolume(); }

    /// Check if gapless playback is supported
    bool preload(input* const i);

    /// Get song info
    const metaData* getMetaData() const { return m_input->getMetaData(); }

    /// Seeking is supported?
    bool seekable() const { return m_input->seekable(); }

    /// Gapless plaback is supported?
    bool gapless() const { return m_input->gapless(); }

    /// Get number of subtunes
    unsigned int subtunes() const { return m_input->subtunes(); }

    /// Get current subtune
    unsigned int subtune() const { return m_input->subtune(); }

    /// Change subtune
    void changeSubtune(dir_t dir);

    /// Get file name of the loaded song
    QString loadedSong() const { return m_input->songLoaded(); }

    /// Get song duration in milliseconds
    unsigned int songDuration() const { return m_input->songDuration(); }

    /// Try switching to preloaded song
    bool tryPreload(const QString& song);

    /// Load song
    void load(const QString& filename);

    /// Unload song
    void unload() { loaded(nullptr); }

    /// Preload song
    void preload(const QString& filename);
};

#endif
