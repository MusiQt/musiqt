/*
 *  Copyright (C) 2006-2025 Leandro Nini
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
#include "inputFactory.h"
#include "input/input.h"
#include "output/qaudioBackend.h"

#include <QDebug>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QStringList>
#include <QIntValidator>

/*****************************************************************/

const char* sampleTypeString(sample_t sampleType)
{
    switch (sampleType)
    {
    case sample_t::U8:           return "U8";
    case sample_t::S16:          return "S16";
    case sample_t::S24:          return "S24";
    case sample_t::S32:          return "S32";
    case sample_t::SAMPLE_FLOAT: return "FLOAT";
    default:                     return "Unknown";
    }
}

audio::audio() :
    m_iw(new InputWrapper(IFACTORY->get())),
    m_audioOutput(new qaudioBackend()),
    m_state(state_t::STOP)
{
    m_volume = m_settings.value(config::AUDIO_VOLUME, 50).toInt();

    connect(m_audioOutput, &qaudioBackend::songEnded,  this, &audio::songEnded);
    connect(m_audioOutput, &qaudioBackend::audioError, this, &audio::audioError);
}

audio::~audio()
{
    stop();

    m_settings.setValue(config::AUDIO_VOLUME, m_volume);

    delete m_audioOutput;
}

bool audio::play(input* i)
{
    if ((i->songLoaded().isEmpty()) || (m_state == state_t::PLAY))
        return false;

    qDebug() << "audio::play";

    if (!i->rewind())
    {
        throw initError("Error rewinding file");
    }

    audioFormat_t format;
    format.sampleRate = i->samplerate();
    format.channels = i->channels();

    switch (i->precision())
    {
    case sample_t::U8:
    case sample_t::S16:
    case sample_t::S24:
    case sample_t::S32:
    case sample_t::SAMPLE_FLOAT:
        format.sampleType = i->precision();
        break;
    case sample_t::SAMPLE_FIXED:
        switch (SETTINGS->bits())
        {
        case 8:
            format.sampleType = sample_t::U8;
            break;
        case 16:
            format.sampleType = sample_t::S16;
            break;
        default:
            throw initError(QString("Unhandled sample type %1").arg(SETTINGS->bits()));
        }
    }

    qDebug() << "Setting parameters"
        << format.sampleRate << ":" << format.channels << ":" << sampleTypeString(format.sampleType);
    m_iw.reset(new InputWrapper(i));
    connect(m_iw.data(), &InputWrapper::songFinished, this, &audio::songEnded);
    connect(m_iw.data(), &InputWrapper::updateTime,  this, &audio::updateTime);
    connect(m_iw.data(), &InputWrapper::preloadSong, this, &audio::preloadSong);

    int const selectedCard = qaudioBackend::getDevices().indexOf(SETTINGS->card());

    try
    {
        audioFormat_t outputFormat = m_audioOutput->init(selectedCard, format);

        qDebug() << "Output parameters"
            << outputFormat.sampleRate << ":" << outputFormat.channels << ":" << sampleTypeString(outputFormat.sampleType);

        if (!m_iw->setFormat(outputFormat))
        {
            m_audioOutput->close();
            throw initError("Unsupported sample type");
        }
    }
    catch (qaudioBackend::initError const &e)
    {
        throw initError(e.message());
    }

    if (SETTINGS->bs2b() && (i->channels() == 2))
        m_iw->enableBs2b();

    m_audioOutput->setVolume(m_volume);

    // We're ready, start playback
    m_audioOutput->start(m_iw.data());

    m_state = state_t::PLAY;

    return true;
}

void audio::pause()
{
    switch (m_state)
    {
    case state_t::PLAY:
        qDebug() << "Pause";
        m_audioOutput->pause();
        m_state = state_t::PAUSE;
        break;
    case state_t::PAUSE:
        qDebug() << "Unpause";
        m_audioOutput->unpause();
        m_state = state_t::PLAY;
        break;
    case state_t::STOP:
        break;
    }
}

bool audio::stop()
{
    if (m_state == state_t::STOP)
        return false;

    qDebug() << "audio::stop";

    m_audioOutput->stop();

    m_audioOutput->close();

    m_iw->close();

    m_state = state_t::STOP;

    m_iw.reset(new InputWrapper(IFACTORY->get()));

    return true;
}

void audio::setVolume(int vol)
{
    m_volume = vol;
    m_audioOutput->setVolume(m_volume);
}

bool audio::gapless(input* const i) const { return m_iw->tryPreload(i); }

void audio::seek(double pos)
{
    m_audioOutput->pause();
    m_iw->setPosition(pos);
    m_audioOutput->unpause();
}

int audio::getPosition() const { return m_iw->getPosition(); }

void audio::resetPosition() { m_iw->resetPosition(); }

void audio::unload() { m_iw->unload(); }

/*****************************************************************/

audioConfig::audioConfig(QWidget* win) :
    configFrame(win)
{
    matrix()->addWidget(new QLabel(tr("Card"), this));
    QComboBox* cardList = new QComboBox(this);
    matrix()->addWidget(cardList);

    {
        // Get a list of audio devices
        const deviceList_t devices = qaudioBackend::getDevices();
        for (auto device: devices)
            cardList->addItem(device.name.replace("\n", " - "), device.id);

        int deviceCnt = devices.size();
        cardList->setMaxVisibleItems((deviceCnt > 5) ? 5 : deviceCnt);

        // Find configured device in list
        QString card = SETTINGS->card();
        int val = cardList->findData(card);
        if (val >= 0) {
            cardList->setCurrentIndex(val);
        } else {
            // backward compatibility
            val = cardList->findText(card.replace("\n", " - "));
            if (val >= 0)
                cardList->setCurrentIndex(val);
        }
    }

    connect(cardList, QOverload<int>::of(&QComboBox::currentIndexChanged),
        [cardList, this](int val) {
            QString card = cardList->itemData(val).toString();

            qDebug() << "onCmdCard" << card;

            SETTINGS->m_card = card;
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
                SETTINGS->m_bits = 8;
                break;
            case 1:
                SETTINGS->m_bits = 16;
                break;
            }
        }
    );

    matrix()->addWidget(new QLabel(tr("Buffer length (ms)"), this));
    QLineEdit *bufLen = new QLineEdit(this);
    matrix()->addWidget(bufLen);
    bufLen->setText(QString::number(SETTINGS->bufLen()));
    bufLen->setValidator(new QIntValidator(5, 5000, this));

    connect(bufLen, &QLineEdit::editingFinished,
        [bufLen, this]() {
            QString val = bufLen->text();
            unsigned int bLen = val.toUInt();
            if (bLen)
            {
                SETTINGS->m_bufLen = bLen;
            }
        }
    );
}
