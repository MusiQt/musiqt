/*
 *  Copyright (C) 2006-2021 Leandro Nini
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

#include "audio.h"

#include "settings.h"
#include "InputWrapper.h"
#include "input/input.h"
#include "output/qaudioBackend.h"

#include <QDebug>
#include <QLabel>
#include <QComboBox>
#include <QStringList>

/*****************************************************************/

const char* sampleTypeString(sample_t sampleType)
{
    switch (sampleType)
    {
    case sample_t::U8:  return "U8";
    case sample_t::S16: return "S16";
    case sample_t::S24: return "S24";
    case sample_t::S32: return "S32";
    default:            return "Unknown";
    }
}

audio::audio() :
    _state(state_t::STOP)
{
    _volume = settings.value("Audio Settings/volume", 50).toInt();

    audioOutput = new qaudioBackend();
    connect(audioOutput, &qaudioBackend::songEnded, this, &audio::songEnded);
}

audio::~audio()
{
    stop();

    settings.setValue("Audio Settings/volume", _volume);

    delete audioOutput;
}

bool audio::play(input* i)
{
    if ((i->songLoaded().isEmpty()) || (_state == state_t::PLAY))
        return false;

    qDebug() << "audio::play";

    i->rewind(); // TODO check return code

    unsigned int selectedCard = 0;
    QString cardName = SETTINGS->card();
    const QStringList devices = qaudioBackend::getDevices();
    for (int dev=0; dev<devices.size(); dev++)
    {
        if (!cardName.compare(devices[dev]))
        {
            selectedCard = dev;
            break;
        }
    }

    unsigned int sampleRate = i->samplerate();

    sample_t sampleType;
    switch (i->precision())
    {
    case sample_t::U8:
    case sample_t::S16:
    case sample_t::S24:
    case sample_t::S32:
        sampleType = i->precision();
        break;
    case sample_t::SAMPLE_FLOAT:
    case sample_t::SAMPLE_FIXED:
        switch (SETTINGS->bits())
        {
        case 8:
            sampleType = sample_t::U8;
            break;
        case 16:
            sampleType = sample_t::S16;
            break;
        default:
            qWarning() << "Unhandled sample type " << SETTINGS->bits();
            return false;
        }
    }

    qDebug() << "Setting parameters " << sampleRate << ":" << i->channels() << ":" << sampleTypeString(sampleType);
    iw = new InputWrapper(i);
    connect(iw, &InputWrapper::switchSong,  this, &audio::songEnded);
    connect(iw, &InputWrapper::updateTime,  this, &audio::updateTime);
    connect(iw, &InputWrapper::preloadSong, this, &audio::preloadSong);
    size_t bufferSize = audioOutput->open(selectedCard, sampleRate, i->channels(), sampleType, iw);
    if (!bufferSize)
        return false;

    qDebug() << "Output samplerate " << sampleRate;
    qDebug() << "bufferSize: " << bufferSize << " bytes";

    iw->setFormat(sampleRate, i->channels(), sampleType, bufferSize);

    if (SETTINGS->bs2b() && (i->channels() == 2))
        iw->enableBs2b();

    audioOutput->volume(_volume);

    // We're ready, resume playback
    audioOutput->unpause();

    _state = state_t::PLAY;

    return true;
}

void audio::pause()
{
    switch (_state)
    {
    case state_t::PLAY:
        qDebug() << "Pause";
        audioOutput->pause();
        _state = state_t::PAUSE;
        break;
    case state_t::PAUSE:
        qDebug() << "Unpause";
        audioOutput->unpause();
        _state = state_t::PLAY;
        break;
    case state_t::STOP:
        break;
    }
}

bool audio::stop()
{
    if (_state == state_t::STOP)
        return false;

    qDebug() << "audio::stop";

    audioOutput->stop();

    iw->close();

    audioOutput->close();

    _state = state_t::STOP;

    delete iw;

    return true;
}

void audio::volume(const int vol)
{
    _volume = vol;
    audioOutput->volume(_volume);
}

bool audio::gapless(input* const i) { return iw->tryPreload(i); }

int audio::seconds() const { return iw->getSeconds(); }

void audio::seek(int pos) { return iw->setPos(pos); }

void audio::unload() { iw->unload(); }

/*****************************************************************/

audioConfig::audioConfig(QWidget* win) :
    configFrame(win, "Audio")
{
    matrix()->addWidget(new QLabel(tr("Card"), this));
    QComboBox* cardList = new QComboBox(this);
    matrix()->addWidget(cardList);

    {
        const QStringList deviceNames = qaudioBackend::getDevices();
        int devices = deviceNames.size();
        cardList->addItems(deviceNames);
        cardList->setMaxVisibleItems((devices > 5) ? 5 : devices);

        int val = cardList->findText(SETTINGS->card());
        if (val >= 0)
            cardList->setCurrentIndex(val);
    }

    connect(cardList, QOverload<int>::of(&QComboBox::currentIndexChanged),
        [cardList, this](int val) {
            QString card = cardList->itemText(val);

            qDebug() << "onCmdCard" << card;

            SETTINGS->_card = card;
        }
    );

    matrix()->addWidget(new QLabel(tr("Default bitdepth"), this));
    QComboBox *bitBox = new QComboBox(this);
    matrix()->addWidget(bitBox);
    QStringList items;
    items << "8" << "16";
    bitBox->addItems(items);
    bitBox->setMaxVisibleItems(items.size());

    {
        unsigned int val;
        switch (SETTINGS->bits())
        {
        case 8:
            val = 0;
            break;
        default:
        case 16:
            val = 1;
            break;
        }

        bitBox->setCurrentIndex(val);
    }

    connect(bitBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
        [this](int val) {
            switch (val)
            {
            case 0:
                SETTINGS->_bits = 8;
                break;
            case 1:
                SETTINGS->_bits = 16;
                break;
            }
        }
    );
}
