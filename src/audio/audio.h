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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#ifdef HAVE_BS2B
#  include <bs2b.h>
#endif

#include "inputTypes.h"

#include <QSettings>
#include <QThread>

class input;
class output;
class converter;

/*****************************************************************/

class audioThread;

/*****************************************************************/

enum class state_t { STOP, PLAY, PAUSE };

class audio : public QObject
{
    Q_OBJECT

    friend class audioThread;

private:
    QSettings   settings;
    input *_input;
    input *_preload;
    output *_output;

    state_t _state;
    bool _playing;

    audioThread *_thread;

    size_t _bufferSize;

    int _buffers;
    int _bufPerSec;
    int _seconds;

    int _volume;

    converter *_converter;
#ifdef HAVE_BS2B
    t_bs2bdp _bs2bdp;
#endif
private:
    audio(const audio&);
    audio& operator=(const audio&);

    sample_t outputPrecision();

    template<typename T>
    void process(size_t size);

    ///
    template<typename T>
    void loop();

signals:
    void songEnded();
    void outputError();
    void updateTime();
    void preloadSong();

public:
    audio();
    ~audio();

    /// Start stream
    bool play(input* i, int pos=0);

    /// Pause stream
    void pause();

    /// Stop stream
    bool stop();

    /// Get state
    state_t state() const { return _state; }

    /// Get seconds
    int seconds() const { return _seconds; }

    /// Set volume
    void volume(const int vol);

    /// Get volume
    int volume() const { return _volume; }

    /// Check if gapless playback is supported
    bool gapless(input* const i);
};

/*****************************************************************/

#include "configFrame.h"

class QComboBox;

class audioConfig : public configFrame
{
    Q_OBJECT

private:
    QComboBox *_cardList;

private:
    audioConfig() {}
    audioConfig(const audioConfig&);
    audioConfig& operator=(const audioConfig&);

    void setCards(const output* const audio);

private slots:
    void onCmdCard(const QString &);
    void onCmdBits(int val);

public:
    audioConfig(QWidget* win);
    virtual ~audioConfig() {}
};

/*****************************************************************/

class audioThread : public QThread
{
    Q_OBJECT

private:
    audio *_audio;

protected:
    void run() override;

public:
    audioThread(audio* a) :
        QThread(),
        _audio(a) {}
    ~audioThread() {}
};

#endif
