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

#include "audio.h"

#include "settings.h"
#include "converter/converterFactory.h"
#include "input/input.h"
#include "output/output.h"
#include "output/outputFactory.h"

#include <QDebug>
#include <QLabel>
#include <QComboBox>
#include <QStringList>

#define DECIMALS 10
//#define PROFILE

#if defined(PROFILE) && QT_VERSION >= 0x040700
#  include <QElapsedTimer>

#  define PROFILE_START QElapsedTimer timer; \
    timer.start();
#  define PROFILE_END   qDebug() << "Elapsed " << timer.elapsed();
#else
#  define PROFILE_START
#  define PROFILE_END
#endif

/*****************************************************************/

void audioThread::run()
{
    switch (_audio->outputPrecision())
    {
    case sample_t::U8:
        _audio->loop<quint8>();
        break;
    case sample_t::S16:
        _audio->loop<qint16>();
        break;
    case sample_t::S24:
    case sample_t::S32:
        qWarning() << "Not supported yet";
        break;
    }
}

/*****************************************************************/

inline sample_t audio::outputPrecision()
{
    switch (_input->precision())
    {
    case sample_t::U8:
    case sample_t::S16:
    case sample_t::S24:
    case sample_t::S32:
        return _input->precision();
    case sample_t::SAMPLE_FLOAT:
    case sample_t::SAMPLE_FIXED:
        switch (SETTINGS->bits())
        {
        case 8:
            return sample_t::U8;
        case 16:
            return sample_t::S16;
        }
    }
}

template<> inline void audio::process<quint8>(size_t size)
{
#if defined HAVE_BS2B && BS2B_VERSION_MAJOR == 3
    if (_bs2bdp)
    {
        uint8_t *buf = (uint8_t*)_output->buffer();
        bs2b_cross_feed_u8(_bs2bdp, buf, size/2);
    }
#endif
}

template<> inline void audio::process<short>(size_t size)
{
#if (Q_BYTE_ORDER == Q_BIG_ENDIAN)
    {
        //Swap bytes on big endian machines
        qint16 *buf = (qint16*)_output->buffer();
        const short *end = buf+(size/2);
        do {
            const qint16 tmp = *buf;
            *buf++ = ((tmp & 0x00FF)<<8) & ((tmp & 0xFF00)>>8);
        } while (buf<end);
    }
#endif

#ifdef HAVE_BS2B
    if (_bs2bdp)
    {
#if BS2B_VERSION_MAJOR == 3
        int16_t *buf = (int16_t*)_output->buffer();
        bs2b_cross_feed_s16le(_bs2bdp, buf, size/4);
#else
        short *buf = (short*)_output->buffer();
        const short *end = buf+(size/2);
        do {
            bs2b_cross_feed_16(_bs2bdp, buf);
            buf += 2;
        } while (buf < end);
#endif
    }
#endif
}

template<typename T>
void audio::loop()
{
    qDebug() << "loop " << static_cast<int>(sizeof(T)<<3) << " bit";
    while (_playing)
    {
PROFILE_START
        size_t size = _input->fillBuffer(
            _converter != nullptr ? _converter->buffer() : _output->buffer(),
            _converter != nullptr ? _converter->bufSize() : _bufferSize,
            _seconds);

        if (!size)
        {
            emit songEnded();
            if (_preload)
            {
                _input = _preload;
                _preload = nullptr;
                _seconds = 0;
                continue;
            }
            else
                break;
        }

        if (_converter != nullptr)
            size = _converter->convert(_output->buffer(), _bufferSize);

        process<T>(size);
PROFILE_END

        if (!_output->write(_output->buffer(), _bufferSize) && _playing)
        {
            qWarning() << "Output error";
            emit outputError();
            _playing = false;
            break;
        }

        _buffers -= (1<<DECIMALS);
        if (_buffers < 0)
        {
            do {
                _buffers += _bufPerSec;
                _seconds++;
            } while (_buffers < 0);
            if (_seconds != _input->time()-5)
                emit updateTime();
            else
                emit preloadSong();
        }
    }
}

/*****************************************************************/

audio::audio() :
    _input(nullptr),
    _preload(nullptr),
    _state(state_t::STOP),
    _playing(false),
    _seconds(0)
#ifdef HAVE_BS2B
    ,_bs2bdp(0)
#endif
{
    _volume = settings.value("Audio Settings/volume", 50).toInt();

    qDebug() << "apiName: " << SETTINGS->apiName();
    _output = OFACTORY->get(SETTINGS->apiName());
}

audio::~audio()
{
    stop();

    settings.setValue("Audio Settings/volume", _volume);

    delete _output;
}

bool audio::play(input* i, int pos)
{
    if ((!_output) || (i->songLoaded().isEmpty()) || (_state == state_t::PLAY))
        return false;

    qDebug() << "audio::play";

    _input = i;

    unsigned int _card = 0;
    QString card = SETTINGS->card();
    for (unsigned int i=0; i<_output->devices(); i++)
    {
        if (!card.compare(_output->device(i)))
        {
            _card = i;
            break;
        }
    }

    unsigned int sampleRate = _input->samplerate();

    // FIXME only supports 8/16 bits
    const unsigned int precision = (outputPrecision() == sample_t::U8) ? 1 : 2;
    qDebug() << "Setting parameters " << sampleRate << ":" << _input->channels() << ":" << precision;
    _bufferSize = _output->open(_card, sampleRate, _input->channels(), precision);
    if (!_bufferSize)
        return false;

    qDebug() << "Output samplerate " << sampleRate;

    // Check if soundcard supports requested samplerate
    _converter = CFACTORY->get(_input->samplerate(), sampleRate, _bufferSize,
        _input->channels(), _input->precision(), outputPrecision(), _input->fract());

    _bufPerSec = ((sampleRate << DECIMALS) * _input->channels() * precision) / _bufferSize;
    _buffers = _bufPerSec;
#ifdef HAVE_BS2B
    if (SETTINGS->bs2b() && (_input->channels() == 2)
#if BS2B_VERSION_MAJOR == 2
        && (outputPrecision() == sample_t::S16)
#endif
    )
    {
        _bs2bdp = bs2b_open();
        if (_bs2bdp)
        {
            qDebug() << "bs2b enabled";
            bs2b_set_srate(_bs2bdp, sampleRate);
            bs2b_set_level(_bs2bdp, BS2B_DEFAULT_CLEVEL);
        }
    }
#endif
    _output->volume(_volume);

    _input->seek(pos);

    _thread = new audioThread(this);

    _playing = true;
    _state = state_t::PLAY;
    _thread->start();

    return true;
}

void audio::pause()
{
    switch (_state)
    {
    case state_t::PLAY:
        qDebug() << "Pause";
        _playing = false;
        _output->pause();
        _thread->wait();
        _state = state_t::PAUSE;
        break;
    case state_t::PAUSE:
        qDebug() << "Unpause";
        _playing = true;
        _thread->start();
        _state = state_t::PLAY;
        break;
    }
}

bool audio::stop()
{
    if ((!_output) || (_state == state_t::STOP))
        return false;

    qDebug() << "audio::stop";

    _playing = false;

    _output->stop();

    _thread->wait();

    _output->close();

    delete _thread;

    _seconds = 0;
    _state = state_t::STOP;
#ifdef HAVE_BS2B
    if (_bs2bdp)
    {
        bs2b_close(_bs2bdp);
        _bs2bdp = 0;
    }
#endif
    if (_converter != nullptr)
        delete _converter;

    return true;
}

void audio::volume(const int vol)
{
    _volume = vol;
    if (_output != nullptr)
        _output->volume(_volume);
}

bool audio::gapless(input* const i)
{
    if ((_input->samplerate() == i->samplerate())
        && (_input->channels() == i->channels())
        && (_input->precision() == i->precision()))
    {
        _preload = i;
        return true;
    }
    else
    {
        _preload = nullptr;
        return false;
    }
}

void audio::setOpts()
{
    delete _output;
    _output = OFACTORY->get(SETTINGS->apiName());
}

/*****************************************************************/

audioConfig::audioConfig(QWidget* win) :
    configFrame(win, "Audio")
{
    matrix()->addWidget(new QLabel(tr("API"), this), 0, 0);
    _apiList = new QComboBox(this);
    matrix()->addWidget(_apiList, 0, 1);
    const int apis = OFACTORY->num();
    for (int i=0; i<apis; i++)
        _apiList->addItem(OFACTORY->name(i));
    _apiList->setMaxVisibleItems(apis);
    int item = _apiList->findText(SETTINGS->_apiName);
    qDebug() << "API " << SETTINGS->_apiName << " - " << item;
    // Fallback to default if not found
    if (item == -1)
    {
        item = 0;
        SETTINGS->_apiName = _apiList->itemText(0);
    }
    _apiList->setCurrentIndex(item);
    connect(_apiList, SIGNAL(currentIndexChanged(int)), this, SLOT(onCmdApi(int)));

    matrix()->addWidget(new QLabel(tr("Card"), this));
    _cardList = new QComboBox(this);
    matrix()->addWidget(_cardList);
    setCards(OFACTORY->get(SETTINGS->_apiName));
    connect(_cardList, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(onCmdCard(const QString &)));

    matrix()->addWidget(new QLabel(tr("Default bitdepth"), this));
    QComboBox *bitBox = new QComboBox(this);
    matrix()->addWidget(bitBox);
    QStringList items;
    items << "8" << "16";
    bitBox->addItems(items);
    bitBox->setMaxVisibleItems(2);

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

    connect(bitBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onCmdBits(int)));
}

void audioConfig::onCmdApi(int val)
{
    qDebug() << "onCmdApi" << val;

    setCards(OFACTORY->get(val));

    SETTINGS->_apiName = _apiList->itemText(val);
}

void audioConfig::onCmdCard(const QString &card)
{
    qDebug() << "onCmdCard" << card;

    SETTINGS->_card = card;
}

void audioConfig::setCards(const output* const audio)
{
    if (audio == nullptr)
        return;

    _cardList->clear();

    const int devices = audio->devices();
    qDebug() << "Devices: " << devices;
    for (int i=0; i<devices; i++)
    {
        _cardList->addItem(audio->device(i));
    }

    delete audio;

    _cardList->setMaxVisibleItems((devices > 5) ? 5 : devices);

    int val = _cardList->findText(SETTINGS->card());
    if (val >= 0)
        _cardList->setCurrentIndex(val);
}

void audioConfig::onCmdBits(int val)
{
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
