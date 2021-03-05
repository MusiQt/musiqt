/*
 *  Copyright (C) 2006-2017 Leandro Nini
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

#ifndef AUDIO_OUTPUT_H
#define AUDIO_OUTPUT_H

#include "inputTypes.h"

#include <QSettings>

class input;
class InputWrapper;
class qaudioBackend;

/*****************************************************************/

enum class state_t { STOP, PLAY, PAUSE };

class audio : public QObject
{
    Q_OBJECT

private:
    QSettings   settings;
    InputWrapper *iw;
    qaudioBackend *audioOutput;

    state_t _state;

    int _volume;

private:
    audio(const audio&);
    audio& operator=(const audio&);

signals:
    void songEnded();
    void outputError();
    void updateTime();
    void preloadSong();

public:
    audio();
    ~audio();

    /// Start stream
    bool play(input* i);

    /// Pause stream
    void pause();

    /// Stop stream
    bool stop();

    /// Get state
    state_t state() const { return _state; }

    /// Get current position in seconds
    int seconds() const;

    /// Set volume
    void volume(const int vol);

    /// Get volume
    int volume() const { return _volume; }

    /// Check if gapless playback is supported
    bool gapless(input* const i);

    void unload();

    /// Set current position
    void seek(int pos);
};

/*****************************************************************/

#include "configFrame.h"

class audioConfig : public configFrame
{
    Q_OBJECT

private:
    audioConfig() {}
    audioConfig(const audioConfig&);
    audioConfig& operator=(const audioConfig&);

    void setCards();

public:
    audioConfig(QWidget* win);
    virtual ~audioConfig() {}
};

#endif
