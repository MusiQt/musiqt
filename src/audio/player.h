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

signals:
    void playbackStopped();
    void subtunechanged();

    void updateTime();
    void songEnded();
    void preloadSong();

public:
    player();
    ~player() {}

    /// Start stream
    bool play() { return m_audio->play(m_input.data()); }

    /// Pause stream
    void pause() { m_audio->pause(); }

    /// Stop stream
    void stop();

    /// Get state
    state_t state() const { return m_audio->state(); }

    /// Get current position in seconds
    int seconds() const { return m_audio->seconds(); }

    /// Set current position
    void seek(int pos) { return m_audio->seek(pos); }

    /// Get position in percent
    int getPosition() const { return m_audio->getPosition(); }

    /// Set volume
    void volume(int vol) { m_audio->volume(vol); }

    /// Get volume
    int volume() const { return m_audio->volume(); }

    /// Check if gapless playback is supported
    bool preload(input* const i);

    /// Get song info
    const metaData* getMetaData() const { return m_input->getMetaData(); }

    /// Seek support
    bool seekable() const { return m_input->seekable(); }

    /// Gapless support
    bool gapless() const { return m_input->gapless(); }

    /// Get number of subtunes
    unsigned int subtunes() const { return m_input->subtunes(); }

    /// Get current subtune
    unsigned int subtune() const { return m_input->subtune(); }

    /// Change subtune
    //bool subtune(unsigned int i) { return m_input->subtune(i); }
    void changeSubtune(dir_t dir);

    /// Song loaded
    QString loadedSong() const { return m_input->songLoaded(); }

    /// Get song duration in milliseconds
    unsigned int songDuration() const { return m_input->songDuration(); }

    bool tryPreload(const QString& song);

    void loaded(input* res, bool subtunes);
    void preloaded(input* res, bool subtunes);
};

#endif
