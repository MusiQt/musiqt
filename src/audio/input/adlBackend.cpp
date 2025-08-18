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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "adlBackend.h"

#include "settings.h"
#include "utils.h"

#include <QComboBox>
#include <QDebug>
#include <QDial>
#include <QFileDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QGroupBox>

#include <string>

#define EXT "mid"

#define CREDITS "libADLMIDI library<br>Copyright \302\251 Vitaly Novichkov."
#define LINK    "https://github.com/Wohlstand/libADLMIDI/"

const char adlBackend::name[] = "ADL";

adlConfig_t adlConfig::m_settings;

QStringList adlBackend::m_ext;

inputConfig* adlBackend::cFactory() { return new adlConfig(name); }

/*****************************************************************/

size_t adlBackend::fillBuffer(void* buffer, const size_t bufferSize)
{
    int samples_count = bufferSize/m_format.containerSize;
    m_buffer.resize(bufferSize);
    ADL_UInt8 *buf = m_buffer.data();
    samples_count = adl_playFormat(m_player, samples_count,
                                   buf,
                                   buf + m_format.containerSize,
                                   &m_format);
    if(samples_count <= 0)
    {
        qWarning() << "Error:";
        return 0;
    }

    std::memcpy(buffer, buf, samples_count * m_format.containerSize);
    return bufferSize;
}

/*****************************************************************/

void adlConfig::loadSettings()
{
    m_settings.samplerate = load("Samplerate", 48000);
}

void adlConfig::saveSettings()
{
    save("Samplerate", m_settings.samplerate);
}

/*****************************************************************/

bool adlBackend::init()
{
    m_ext = QString(EXT).split("|");
    return true;
}

QStringList adlBackend::ext() { return m_ext; }

adlBackend::adlBackend(const QString& fileName) :
    m_currentTrack(0)
    , m_config(name)
{
    m_player = adl_init(m_config.samplerate());

    if (m_player == nullptr)
    {
        qWarning() << "Warning: " << adl_errorString();
        throw loadError("Error creating gme emu");
    }

    int err = adl_openFile(m_player, fileName.toUtf8().constData());
    if (err < 0)
    {
        qWarning() << "Warning: " <<  adl_errorInfo(m_player);
        throw loadError("Error creating gme emu");
    }

    m_format.type = ADLMIDI_SampleType_S16;
    m_format.containerSize = sizeof(int16_t);
    m_format.sampleOffset = sizeof(int16_t) * 2;

    QString title = QString::fromLatin1(adl_metaMusicTitle(m_player));
    m_metaData.addInfo(metaData::TITLE, title);

    m_subtunes = adl_getSongsCount(m_player);

    setDuration(adl_totalTimeLength(m_player) * 1000.);

    songLoaded(fileName);
}

adlBackend::~adlBackend()
{
    adl_close(m_player);
}

bool adlBackend::rewind()
{
    adl_positionRewind(m_player);
    return true;
}

bool adlBackend::subtune(unsigned int i)
{
    if ((i > 0) && (i <= m_subtunes))
    {
        m_currentTrack = i - 1;
        adl_selectSongNum(m_player, m_currentTrack);

        return true;
    }

    qWarning() << "Invalid subtune" << i;
    return false;
}

/*****************************************************************/

#include "iconFactory.h"

#define ADLSETTINGS adlConfig::m_settings

adlConfigFrame::adlConfigFrame(QWidget* win) :
    configFrame(win, CREDITS, LINK)
{
    matrix()->addWidget(new QLabel(tr("Samplerate"), this), 0, 0);
    QComboBox *freqBox = new QComboBox(this);
    matrix()->addWidget(freqBox, 0, 1);
    {
        QStringList items;
        items << "11025" << "22050" << "44100" << "48000";
        freqBox->addItems(items);
        freqBox->setMaxVisibleItems(items.size());
    }

    int val;
    switch (ADLSETTINGS.samplerate)
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
        [this](int val) {
            switch (val)
            {
            case 0:
                ADLSETTINGS.samplerate = 11025;
                break;
            case 1:
                ADLSETTINGS.samplerate = 22050;
                break;
            case 2:
                ADLSETTINGS.samplerate = 44100;
                break;
            case 3:
                ADLSETTINGS.samplerate = 48000;
                break;
            }
        }
    );
}
