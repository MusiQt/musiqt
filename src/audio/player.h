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

#include "audioState.h"
#include "metaData.h"

#include <QObject>
#include <QScopedPointer>

#include <memory>

class audio;
class input;

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
    QScopedPointer<audio> m_audio;
    std::unique_ptr<input> m_preload;

private:
    player(const player&) = delete;
    player& operator=(const player&) = delete;

    void loaded(input* res);

    void preloaded(input* res);

    void onError(const QString& error);

signals:
    void stateChanged();
    void songChanged();

    void subtunechanged();

    void updateTime();
    void songEnded();
    void preloadSong();

    void songLoaded(bool loaded);
    void volumeChanged();
    void positionChanged();

    void audioError(const QString&);

public:
    player();
    ~player() override;

    /// Start playback
    void play();

    /// Pause/unpause playback
    void pause();

    /// Stop playback
    void stop();

    /// Get playback state
    state_t state() const;

    /// Get current position in seconds
    int seconds() const;

    /// Set current position [0,1]
    void setPosition(double pos);

    /// Get current position in percent
    int getPosition() const;

    /// Set volume
    void setVolume(int vol);

    /// Get volume
    int getVolume() const;

    /// Check if gapless playback is supported
    bool preload(input* const i);

    /// Get song info
    const metaData* getMetaData() const;

    /// Seeking is supported?
    bool seekable() const;

    /// Gapless plaback is supported?
    bool gapless() const;

    /// Get number of subtunes
    unsigned int subtunes() const;

    /// Get current subtune
    unsigned int subtune() const;

    /// Change subtune
    void changeSubtune(dir_t dir);

    /// Get file name of the loaded song
    QString loadedSong() const;

    /// Get song duration in milliseconds
    unsigned int songDuration() const;

    /// Try switching to preloaded song
    bool tryPreload(const QString& song);

    /// Load song
    void load(const QString& filename);

    /// Start playback once song is loaded
    void playOnLoad();

    /// Unload song
    void unload() { loaded(nullptr); }

    /// Preload song
    void preload(const QString& filename);
};

#endif
