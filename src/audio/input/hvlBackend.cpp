/*
 *  Copyright (C) 2007-2017 Leandro Nini
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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "hvlBackend.h"

#include "settings.h"
#include "utils.h"

#include <QDebug>
#include <QComboBox>

#define EXT "ahx|hvl" // FIXME support as prefix (Amiga style) too

#define CREDITS "HivelyTracker\nDexter/Abyss, Xeron/IRIS, and Pieknyman"
#define LINK    "https://github.com/pete-gordon/hivelytracker"

const char hvlBackend::name[] = "Hively";

hvlConfig_t hvlBackend::_settings;

/*****************************************************************/

size_t hvlBackend::fillBuffer(void* buffer, const size_t bufferSize, const unsigned int seconds)
{
    size_t offset=0;

    if (_left)
    {
        memcpy(buffer, _buffer+_size-_left, _left);
        offset = _left;
        _left = 0;
    }

    do
    {
        hvl_DecodeFrame(_tune, ((int8*)buffer)+offset, ((int8*)buffer)+offset+2, 4);
        offset += _size;
    } while (offset<bufferSize-_size);

    size_t left=bufferSize-offset;
    if (left)
    {
        hvl_DecodeFrame(_tune, (int8*)_buffer, ((int8*)_buffer)+2, 4);
        memcpy((char*)buffer+offset, _buffer, left);
        _left = _size-left;
    }

    return bufferSize;
}

/*****************************************************************/

bool hvlBackend::supports(const QString& fileName)
{
    QString ext(EXT);
    ext.prepend(".*\\.(").append(")");
    qDebug() << "hvlBackend::supports: " << ext;

    QRegExp rx(ext);
    return rx.exactMatch(fileName);
}

inline QStringList hvlBackend::ext() const { return QString(EXT).split("|"); }

hvlBackend::hvlBackend() :
    inputBackend(name),
    _tune(0),
    _buffer(0)
{
    loadSettings();

    hvl_InitReplayer();
}

hvlBackend::~hvlBackend()
{
    close();
}

void hvlBackend::loadSettings()
{
    _settings.samplerate = load("Samplerate", 44100);
}

void hvlBackend::saveSettings()
{
    save("Samplerate", _settings.samplerate);
}

bool hvlBackend::open(const QString& fileName)
{
    close();

    _tune = hvl_LoadTune((TEXT*)fileName.toLocal8Bit().constData(), _settings.samplerate, 4);

    if (_tune == nullptr)
        return false;

    hvl_InitSubsong(_tune, 0);

    _metaData.addInfo(metaData::TITLE, (char*)_tune->ht_Name);
    QString comment = QString::null;
    for (unsigned int i=0; i<_tune->ht_InstrumentNr; i++)
        comment.append(QString("%1").arg(_tune->ht_Instruments[i].ins_Name));

    _metaData.addInfo(metaData::COMMENT, comment.trimmed());

    _left = 0;
    _size = (_settings.samplerate*4) / 50;
    _buffer = new char[_size];

    songLoaded(fileName);
    return true;
}

void hvlBackend::close()
{
    if (_tune)
    {
        hvl_FreeTune(_tune);
        _tune = nullptr;
        delete[] _buffer;
        _buffer = nullptr;
    }

    songLoaded(QString::null);
}

bool hvlBackend::rewind()
{
    if (_tune != nullptr)
    {
        hvl_InitSubsong(_tune, _tune->ht_SongNum);
        return true;
    }
    return false;
}

bool hvlBackend::subtune(const unsigned int i)
{
    if (_tune != nullptr)
    {
        hvl_InitSubsong(_tune, i);
        return true;
    }
    return false;
}

/*****************************************************************/

#define HVLSETTINGS hvlBackend::_settings

hvlConfig::hvlConfig(QWidget* win) :
    configFrame(win, hvlBackend::name, CREDITS, LINK)
{
    matrix()->addWidget(new QLabel(tr("Samplerate"), this));
    QComboBox *freqBox = new QComboBox(this);
    matrix()->addWidget(freqBox);
    QStringList items;
    items << "11025" << "22050" << "44100" << "48000";
    freqBox->addItems(items);
    freqBox->setMaxVisibleItems(4);

    int val;
    switch (HVLSETTINGS.samplerate)
    {
    case 11025:
        val = 0;
        break;
    case 22050:
        val = 1;
        break;
    default:
    case 44100:
        val = 2;
        break;
    case 48000:
        val = 3;
        break;
    }

    freqBox->setCurrentIndex(val);

    connect(freqBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onCmdSamplerate(int)));
}

void hvlConfig::onCmdSamplerate(int val)
{
    switch (val)
    {
    case 0:
        HVLSETTINGS.samplerate = 11025;
        break;
    case 1:
        HVLSETTINGS.samplerate = 22050;
        break;
    case 2:
        HVLSETTINGS.samplerate = 44100;
        break;
    case 3:
        HVLSETTINGS.samplerate = 48000;
        break;
    }
}
