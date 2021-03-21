/*
 *  Copyright (C) 2007-2021 Leandro Nini
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

extern const unsigned char iconHvl[1006] =
{
    0x47, 0x49, 0x46, 0x38, 0x39, 0x61, 0x10, 0x00, 0x10, 0x00, 0xf7, 0x87, 0x00, 0x18, 0x26, 0x34,
    0x1a, 0x27, 0x35, 0x1e, 0x29, 0x34, 0x1c, 0x2c, 0x3d, 0x1f, 0x2c, 0x3c, 0x1e, 0x2d, 0x3c, 0x1f,
    0x2d, 0x3c, 0x1f, 0x2e, 0x3d, 0x1f, 0x2f, 0x3e, 0x22, 0x26, 0x2a, 0x24, 0x27, 0x2d, 0x23, 0x29,
    0x2e, 0x25, 0x2a, 0x2d, 0x25, 0x2d, 0x37, 0x2c, 0x2e, 0x30, 0x20, 0x2f, 0x3e, 0x2f, 0x32, 0x36,
    0x27, 0x30, 0x3b, 0x21, 0x30, 0x3d, 0x2a, 0x32, 0x39, 0x2f, 0x35, 0x3b, 0x2d, 0x33, 0x3c, 0x2d,
    0x36, 0x3c, 0x2e, 0x34, 0x3e, 0x31, 0x36, 0x39, 0x33, 0x38, 0x3b, 0x35, 0x38, 0x3b, 0x32, 0x38,
    0x3e, 0x33, 0x38, 0x3e, 0x35, 0x38, 0x3c, 0x36, 0x38, 0x3c, 0x35, 0x3a, 0x3f, 0x38, 0x3a, 0x3d,
    0x38, 0x3a, 0x3e, 0x21, 0x31, 0x41, 0x24, 0x32, 0x40, 0x24, 0x32, 0x41, 0x26, 0x34, 0x41, 0x23,
    0x35, 0x46, 0x25, 0x36, 0x46, 0x25, 0x37, 0x49, 0x25, 0x37, 0x4a, 0x25, 0x38, 0x4a, 0x25, 0x38,
    0x4b, 0x26, 0x3a, 0x4d, 0x27, 0x3b, 0x4e, 0x2d, 0x3b, 0x49, 0x2e, 0x3d, 0x4f, 0x33, 0x3a, 0x43,
    0x32, 0x3a, 0x45, 0x30, 0x3e, 0x51, 0x3b, 0x40, 0x45, 0x3d, 0x43, 0x49, 0x3e, 0x43, 0x48, 0x3c,
    0x42, 0x4a, 0x3d, 0x43, 0x4b, 0x3e, 0x47, 0x4f, 0x34, 0x43, 0x55, 0x36, 0x56, 0x77, 0x37, 0x56,
    0x77, 0x37, 0x57, 0x78, 0x38, 0x58, 0x79, 0x39, 0x58, 0x79, 0x38, 0x5a, 0x7d, 0x39, 0x5b, 0x7e,
    0x39, 0x5c, 0x7f, 0x3a, 0x5c, 0x7f, 0x4d, 0x55, 0x63, 0x4e, 0x59, 0x67, 0x51, 0x5b, 0x6b, 0x52,
    0x5c, 0x6a, 0x52, 0x5c, 0x6b, 0x3b, 0x5d, 0x80, 0x3c, 0x5e, 0x81, 0x40, 0x63, 0x84, 0x42, 0x63,
    0x85, 0x40, 0x63, 0x86, 0x43, 0x64, 0x85, 0x43, 0x64, 0x86, 0x44, 0x66, 0x87, 0x41, 0x66, 0x8b,
    0x44, 0x66, 0x88, 0x44, 0x66, 0x89, 0x44, 0x67, 0x89, 0x45, 0x67, 0x89, 0x45, 0x68, 0x8a, 0x46,
    0x68, 0x8a, 0x47, 0x68, 0x8a, 0x47, 0x69, 0x8a, 0x47, 0x69, 0x8b, 0x47, 0x69, 0x8c, 0x48, 0x6a,
    0x8d, 0x46, 0x6a, 0x90, 0x47, 0x6c, 0x91, 0x48, 0x6c, 0x90, 0x48, 0x6c, 0x91, 0x48, 0x6d, 0x91,
    0x48, 0x6c, 0x92, 0x48, 0x6d, 0x92, 0x49, 0x6e, 0x93, 0x68, 0x7a, 0x8f, 0x76, 0x84, 0x94, 0x7e,
    0x8c, 0x9c, 0x83, 0x92, 0xa2, 0x8f, 0x9c, 0xa8, 0x80, 0x99, 0xb5, 0x85, 0x9d, 0xbb, 0x9b, 0xa4,
    0xaa, 0x8f, 0xa5, 0xbf, 0x99, 0xa4, 0xb0, 0x97, 0xa9, 0xbd, 0xa5, 0xb2, 0xbd, 0xba, 0xb8, 0xb6,
    0x9b, 0xae, 0xc2, 0xa3, 0xb5, 0xc5, 0xa7, 0xb6, 0xc7, 0xbe, 0xc4, 0xc7, 0xb7, 0xc4, 0xd0, 0xb9,
    0xc6, 0xd3, 0xba, 0xc7, 0xd4, 0xb4, 0xc6, 0xd8, 0xc2, 0xc1, 0xc0, 0xc2, 0xc6, 0xc6, 0xcb, 0xc9,
    0xc6, 0xd1, 0xce, 0xca, 0xd2, 0xd4, 0xd1, 0xdb, 0xd8, 0xd4, 0xd6, 0xdc, 0xdd, 0xe5, 0xe2, 0xde,
    0xc4, 0xd2, 0xe0, 0xc4, 0xd4, 0xe3, 0xde, 0xe5, 0xe7, 0xd0, 0xe6, 0xfc, 0xe2, 0xe8, 0xe7, 0xf7,
    0xfa, 0xfc, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x21, 0xf9, 0x04,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x2c, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x10, 0x00, 0x00, 0x08,
    0xcb, 0x00, 0xa3, 0x08, 0x1c, 0x48, 0xb0, 0xe0, 0xc0, 0x27, 0x4f, 0x0c, 0x2a, 0x8c, 0xb2, 0x65,
    0x8b, 0xc0, 0x2d, 0x5a, 0x22, 0x2a, 0x6c, 0xc2, 0x63, 0x47, 0x93, 0x25, 0x3c, 0x7c, 0xf8, 0xe0,
    0xb1, 0xa4, 0x20, 0x18, 0x13, 0x1a, 0x3c, 0xa0, 0x38, 0x11, 0xc2, 0x03, 0x8c, 0x10, 0x07, 0xba,
    0x08, 0x94, 0x22, 0x05, 0xcc, 0x81, 0x3e, 0x7f, 0x06, 0x1c, 0x28, 0x44, 0x47, 0xcf, 0xa0, 0x00,
    0x50, 0x96, 0x30, 0xa9, 0x22, 0xa5, 0xcb, 0x88, 0x36, 0x6e, 0x12, 0x38, 0x98, 0x73, 0x06, 0x4d,
    0x1c, 0x06, 0x13, 0x2c, 0x2c, 0x68, 0xd1, 0x85, 0x8b, 0x8b, 0x1b, 0x32, 0x8c, 0x18, 0xc9, 0xd1,
    0x00, 0xc7, 0x0b, 0x22, 0x43, 0x8a, 0xd8, 0x38, 0x30, 0xa6, 0x4b, 0x09, 0x33, 0x69, 0x1a, 0x54,
    0x60, 0x43, 0xa6, 0x8c, 0x1a, 0x18, 0x16, 0x62, 0x28, 0xe0, 0xda, 0x45, 0x42, 0x1d, 0x3c, 0x01,
    0x04, 0x08, 0x92, 0xf3, 0x86, 0xd0, 0x1d, 0x3b, 0x81, 0xd6, 0x3c, 0x00, 0xe3, 0x92, 0x8f, 0x1f,
    0x11, 0x22, 0x00, 0xe5, 0x81, 0x73, 0xc8, 0x90, 0xa1, 0x43, 0x7b, 0x1e, 0x84, 0x01, 0x83, 0x02,
    0x43, 0x86, 0x16, 0x28, 0x3c, 0x70, 0xc0, 0x70, 0x63, 0xc6, 0x8c, 0x1b, 0x10, 0x56, 0x84, 0x11,
    0x88, 0x04, 0x88, 0x40, 0x21, 0x49, 0x92, 0x00, 0x19, 0x0d, 0x24, 0x09, 0xc1, 0x2c, 0x55, 0x04,
    0x66, 0x41, 0xbd, 0x7a, 0x4b, 0xea, 0x85, 0xb0, 0x63, 0x2b, 0x0c, 0x08, 0x00, 0x3b
};

#define EXT "ahx|hvl" // FIXME support as prefix (Amiga style) too

#define CREDITS "HivelyTracker<br>Version 1.8<br>Dexter/Abyss, Xeron/IRIS, and Pieknyman"
#define LINK    "https://github.com/pete-gordon/hivelytracker"

const char hvlBackend::name[] = "Hively";

hvlConfig_t hvlConfig::m_settings;

inputConfig* hvlBackend::cFactory() { return new hvlConfig(name, iconHvl, 1006); }

/*****************************************************************/

size_t hvlBackend::fillBuffer(void* buffer, const size_t bufferSize)
{
    if (_tune->ht_SongEndReached)
        return 0;

    size_t offset = 0;

    // use what's left in the backing buffer
    if (_left)
    {
        memcpy(buffer, _buffer+_size-_left, _left);
        offset = _left;
        _left = 0;
    }

    // fill output buffer while there's enough space
    size_t bufferSpaceLeft = bufferSize - offset;
    while (bufferSpaceLeft > _size)
    {
        hvl_DecodeFrame(_tune, ((int8*)buffer)+offset, ((int8*)buffer)+offset+2, 4);
        offset += _size;
        bufferSpaceLeft -= _size;
    }

    // use backing buffer to fill the remaining space
    if (bufferSpaceLeft)
    {
        hvl_DecodeFrame(_tune, (int8*)_buffer, ((int8*)_buffer)+2, 4);
        memcpy((char*)buffer+offset, _buffer, bufferSpaceLeft);
        _left = _size - bufferSpaceLeft;
    }

    return bufferSize;
}

/*****************************************************************/

void hvlConfig::loadSettings()
{
    m_settings.samplerate = load("Samplerate", 44100);
}

void hvlConfig::saveSettings()
{
    save("Samplerate", m_settings.samplerate);
}

/*****************************************************************/

QStringList hvlBackend::ext() { return QString(EXT).split("|"); }

hvlBackend::hvlBackend() :
    _tune(nullptr),
    _buffer(nullptr),
    m_config(name, iconHvl, 1006)
{
    hvl_InitReplayer();
}

hvlBackend::~hvlBackend()
{
    if (_tune)
    {
        hvl_FreeTune(_tune);
        delete[] _buffer;
    }
}

bool hvlBackend::open(const QString& fileName)
{
    _tune = hvl_LoadTune((TEXT*)fileName.toUtf8().constData(), m_config.samplerate(), 2);

    if (_tune == nullptr)
        return false;

    hvl_InitSubsong(_tune, 0); // TODO check return value

    m_metaData.addInfo(metaData::TITLE, (char*)_tune->ht_Name);
    QString comment = QString();
    for (unsigned int i=0; i<_tune->ht_InstrumentNr; i++)
        comment.append(QString("%1\n").arg(_tune->ht_Instruments[i].ins_Name));

    m_metaData.addInfo(metaData::COMMENT, comment.trimmed());

    _left = 0;
    _size = (m_config.samplerate()*4) / 50;
    _buffer = new char[_size];

    songLoaded(fileName);
    return true;
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

#define HVLSETTINGS hvlConfig::m_settings

hvlConfigFrame::hvlConfigFrame(QWidget* win) :
    configFrame(win, hvlBackend::name, CREDITS, LINK)
{
    matrix()->addWidget(new QLabel(tr("Samplerate"), this), 0, 0);
    QComboBox *freqBox = new QComboBox(this);
    matrix()->addWidget(freqBox, 0, 1);
    QStringList items;
    items << "11025" << "22050" << "44100" << "48000";
    freqBox->addItems(items);
    freqBox->setMaxVisibleItems(items.size());

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

    connect(freqBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
        [](int val) {
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
    );
}
