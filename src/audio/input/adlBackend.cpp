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

#define EXT "mid|midi"

#define CREDITS "libADLMIDI library<br>Copyright \302\251 Vitaly Novichkov."
#define LINK    "https://github.com/Wohlstand/libADLMIDI/"

const char adlBackend::name[] = "ADL";

adlConfig_t adlConfig::m_settings;

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
    if (samples_count <= 0)
    {
        qDebug() << "Playback error or song finished";
        return 0;
    }

    float *dest = reinterpret_cast<float*>(buffer);
    float *src = reinterpret_cast<float*>(buf);
    // Apply gain
    for (int i=0; i<samples_count; i++)
        dest[i] = src[i] * m_config.gain();

    return samples_count * m_format.containerSize;
}

/*****************************************************************/

void adlConfig::loadSettings()
{
    m_settings.samplerate = load("Samplerate", 48000);
    m_settings.gain = load("Gain", 2.f);
    m_settings.woplPath = load("WOPL bank", QString());
}

void adlConfig::saveSettings()
{
    save("Samplerate", m_settings.samplerate);
    save("Gain", m_settings.gain);
    save("WOPL bank", m_settings.woplPath);
}

/*****************************************************************/

QStringList adlBackend::ext() { return QString(EXT).split("|"); }

adlBackend::adlBackend(const QString& fileName) :
    m_currentTrack(0)
    , m_config(name)
{
    m_player = adl_init(m_config.samplerate());
    if (!m_player)
    {
        QString error = QString("Error: %1").arg(adl_errorString());
        throw loadError(error);
    }

    qInfo() << "Using emulator: " << adl_chipEmulatorName(m_player);

    qDebug() << "Volume model: " << adl_getVolumeRangeModel(m_player);

    if (!m_config.woplPath().isEmpty())
    {
        qDebug() << "Loading bank: " << m_config.woplPath();
        int err = adl_openBankFile(m_player, m_config.woplPath().toUtf8().constData());
        if (err < 0)
        {
            qWarning() << "Warning: " << adl_errorInfo(m_player);
        }
    }

    int err = adl_openFile(m_player, fileName.toUtf8().constData());
    if (err < 0)
    {
        QString error = QString("Error: %1").arg(adl_errorInfo(m_player));
        throw loadError(error);
    }

    m_format.type = ADLMIDI_SampleType_F32;
    m_format.containerSize = sizeof(float);
    m_format.sampleOffset = sizeof(float) * 2;

    QString title = QString::fromUtf8(adl_metaMusicTitle(m_player));
    m_metaData.addInfo(metaData::TITLE, title);
    QString copyright = QString::fromUtf8(adl_metaMusicCopyright(m_player));
    m_metaData.addInfo(gettext("copyright"), copyright);

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

bool adlBackend::seek(double pos)
{
    double len = adl_totalTimeLength(m_player);
    adl_positionSeek(m_player, pos*len);
    return true;
}

/*****************************************************************/

#include "iconFactory.h"

#define ADLSETTINGS adlConfig::m_settings

inline float toDb(float val)
{
    return 20.f * std::log10(val);
}

inline float fromDb(float val)
{
    return std::pow(10.f, val/20.f);
}

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

    QGridLayout *frame = new QGridLayout();
    extraBottom()->addLayout(frame);

    QPushButton* button;

    frame->addWidget(new QLabel(tr("WOPL bank path:"), this), 0, 0);
    QLineEdit *woplPath = new QLineEdit(this);
    woplPath->setText(ADLSETTINGS.woplPath);
    frame->addWidget(woplPath, 0, 1);
    connect(woplPath, &QLineEdit::editingFinished,
        [woplPath, this]() {
            QString val = woplPath->text();
            if (checkPath(val))
            {
                ADLSETTINGS.woplPath = val;
            }
        }
    );
    button = new QPushButton(GET_ICON(icon_documentopen), tr("&Browse"), this);
    button->setToolTip(tr("Select WOPL bank"));
    frame->addWidget(button, 0, 2);
    connect(button, &QPushButton::clicked,
        [woplPath, this]() {
            QString dir = QFileDialog::getOpenFileName(this, tr("Select WOPL bank"), ADLSETTINGS.woplPath, tr("bank (*.wopl)"));
            if (!dir.isNull())
                ADLSETTINGS.woplPath = dir;

            woplPath->setText(ADLSETTINGS.woplPath);
        }
    );

    QHBoxLayout *hf = new QHBoxLayout();
    extraBottom()->addLayout(hf);

    QLabel *tf;

    hf->addStretch();

    QGridLayout *mat = new QGridLayout();
    hf->addLayout(mat);

    hf->addStretch();

    int const dbGain = static_cast<int>(toDb(ADLSETTINGS.gain));

    tf = new QLabel(tr("Gain (dB)"), this);
    tf->setAlignment(Qt::AlignCenter);
    mat->addWidget(tf, 0, 0);
    QDial *knob = new QDial(this);
    knob->setRange(0, 15);
    knob->setValue(dbGain);
    mat->addWidget(knob, 1, 0);
    tf = new QLabel(this);
    tf->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    tf->setAlignment(Qt::AlignCenter);
    tf->setNum(dbGain);
    mat->addWidget(tf, 2, 0);
    connect(knob, &QDial::valueChanged,
        [tf](int val) {
            ADLSETTINGS.gain = fromDb(val);
            tf->setNum(val);
        }
    );
}
