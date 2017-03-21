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

#include "openmptBackend.h"

#include "settings.h"
#include "utils.h"

#include <QComboBox>
#include <QDebug>
#include <QDial>
#include <QFileDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

#include <algorithm>
#include <string>
#include <vector>

#ifdef HAVE_LIBZ
#  include "libs/unzip/unzip.h"
#  ifdef _WIN32
#    include "libs/unzip/iowin32.h"
#  endif
#  define EXTZ "xmz|itz|mdz|s3z"
#endif

#define CREDITS "libopenmpt"
#define LINK    "http://lib.openmpt.org/"

const char openmptBackend::name[] = "Openmpt";

openmptConfig_t openmptBackend::_settings;

QStringList openmptBackend::_ext;

/*****************************************************************/

size_t openmptBackend::fillBuffer(void* buffer, const size_t bufferSize, const unsigned int seconds)
{
    size_t bufSize = (bufferSize/sizeof(float))/_settings.channels;
    return _settings.channels == 2
        ? _module->read_interleaved_stereo(_settings.samplerate, bufSize, (float*)buffer)
        : _module->read(_settings.samplerate, bufSize, (float*)buffer);
}

/*****************************************************************/

bool openmptBackend::init()
{
    std::vector<std::string> ext = openmpt::get_supported_extensions();
    for(std::vector<std::string>::iterator it=ext.begin(); it!=ext.end(); ++it)
    {
        _ext << (*it).c_str();
    }

    //_ext.append(",(mod).*");

#ifdef HAVE_LIBZ
    _ext << QString(EXTZ).split("|");
#endif
    return true;
}

bool openmptBackend::supports(const QString& fileName)
{
    QString ext = _ext.join("|");
    ext.prepend(".*\\.(").append(")");
    qDebug() << "sndBackend::supports: " << ext;

    QRegExp rx(ext);
    return rx.exactMatch(fileName);
}

openmptBackend::openmptBackend() :
    inputBackend(name),
    _module(nullptr)
{
    loadSettings();
}

openmptBackend::~openmptBackend()
{
    close();
}

void openmptBackend::loadSettings()
{
    _settings.samplerate = load("Frequency", 44100);
    _settings.channels = load("Channels", 2);
    _settings.resamplingMode = load("Resampling", 8);
    _settings.masterGain = load("Master gain", 0);
    _settings.stereoSeparation = load("Stereo separation", 40);
    _settings.volumeRamping = load("Volume ramping", 0);
}

void openmptBackend::saveSettings()
{
    save("Frequency", _settings.samplerate);
    save("Channels", _settings.channels);
    save("Resampling", _settings.resamplingMode);
    save("Master gain", _settings.masterGain);
    save("Stereo separation", _settings.stereoSeparation);
    save("Volume ramping", _settings.volumeRamping);
}

bool openmptBackend::open(const QString& fileName)
{
    close();

    bool tmpFile = false;
    QString fName;

#ifdef HAVE_LIBZ
    QString ext(EXTZ);
    ext.prepend(".*\\.(").append(")");
    QRegExp rx(ext);
    if (rx.exactMatch(fileName))
    {
#  ifdef _WIN32
        zlib_filefunc64_def ffunc;
        fill_win32_filefunc64(&ffunc);
#    ifdef UNICODE
        const wchar_t* tmpFileName = convertUtf(fileName);
        unzFile modZip = unzOpen2_64(tmpFileName, &ffunc);
        delete [] tmpFileName;
#    else
        unzFile modZip = unzOpen2_64(fileName.toLocal8Bit().constData(), &ffunc);
#    endif // UNICODE
#  else
        unzFile modZip = unzOpen64(fileName.toLocal8Bit().constData());
#  endif // _WIN32
        if (unzGoToFirstFile(modZip) != UNZ_OK)
        {
            unzClose(modZip);
            return false;
        }

        fName = tempFile(fileName);
        qDebug() << "temp file: " << fName;
        QFile modFile(fName);
        modFile.open(QIODevice::WriteOnly);
        unzOpenCurrentFile(modZip);
        int n;
        char buffer[4096];
        do {
            n = unzReadCurrentFile(modZip, buffer, 4096);
        } while (modFile.write(buffer, n));
        unzCloseCurrentFile(modZip);
        unzClose(modZip);
        tmpFile = true;
    }
#endif // HAVE_LIBZ

    if (!tmpFile)
        fName = fileName;

    QFile f(fName);
    f.open(QIODevice::ReadOnly);
    const long size = f.size();
    char* data = new char[size];
    f.read(data, size);
    f.close();

    void* buffer = (void*)data;
    unsigned int len = size;

    try
    {
        _module = new openmpt::module(buffer, len);
    }
    catch (const openmpt::exception &e)
    {
        delPtr(_module);
        return false;
    }

    delete [] data;

    delTempFile(tmpFile, fName);

    _module->set_render_param(openmpt::module::RENDER_INTERPOLATIONFILTER_LENGTH, _settings.resamplingMode);
    _module->set_render_param(openmpt::module::RENDER_MASTERGAIN_MILLIBEL, _settings.masterGain);
    _module->set_render_param(openmpt::module::RENDER_STEREOSEPARATION_PERCENT, _settings.stereoSeparation);
    _module->set_render_param(openmpt::module::RENDER_VOLUMERAMPING_STRENGTH, _settings.volumeRamping);

    std::vector<std::string> metadata = _module->get_metadata_keys();
    for(std::vector<std::string>::iterator it=metadata.begin(); it!=metadata.end(); ++it)
    {
        qDebug() << (*it).c_str();
        if ((*it).compare("title") == 0)
            _metaData.addInfo(metaData::TITLE, _module->get_metadata("title").c_str());
        else if ((*it).compare("artist") == 0)
            _metaData.addInfo(metaData::ARTIST, _module->get_metadata("artist").c_str());
        else if ((*it).compare("message") == 0)
            _metaData.addInfo(metaData::COMMENT, _module->get_metadata("message").c_str());
    }

    time(_module->get_duration_seconds());

    songLoaded(fileName);
    return true;
}

void openmptBackend::close()
{
    if (_module)
    {
        delPtr(_module);
    }

    songLoaded(QString::null);
}

bool openmptBackend::rewind()
{
    if (_module)
    {
        _module->set_position_order_row(0, 0);
        return true;
    }
    return false;
}

/*****************************************************************/

#define MPTSETTINGS openmptBackend::_settings

openmptConfig::openmptConfig(QWidget* win) :
    configFrame(win, openmptBackend::name, CREDITS, LINK)
{
    int val;

    matrix()->addWidget(new QLabel(tr("Frequency"), this), 0, 0);
    QComboBox *freqBox = new QComboBox(this);
    matrix()->addWidget(freqBox, 0, 1);
    {
        QStringList items;
        items << "11025" << "22050" << "44100" << "48000";
        freqBox->addItems(items);
    }
    freqBox->setMaxVisibleItems(4);
    switch (MPTSETTINGS.samplerate)
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
    connect(freqBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onCmdFrequency(int)));

    matrix()->addWidget(new QLabel(tr("Channels"), this));
    QComboBox *chanBox = new QComboBox(this);
    matrix()->addWidget(chanBox);
    {
        QStringList items;
        items << "Mono" << "Stereo";
        chanBox->addItems(items);
    }
    chanBox->setMaxVisibleItems(2);
    chanBox->setCurrentIndex(MPTSETTINGS.channels-1);
    connect(chanBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onCmdChannels(int)));

    matrix()->addWidget(new QLabel(tr("Resampling"), this));
    QComboBox *resBox = new QComboBox(this);
    matrix()->addWidget(resBox);
    {
        QStringList items;
        items << "Zero order hold" << "Linear" << "Cubic Spline" << "Sinc 8 taps";
        resBox->addItems(items);
    }
    resBox->setMaxVisibleItems(4);
    switch (MPTSETTINGS.resamplingMode)
    {
    case 1:
        val = 0;
        break;
    case 2:
        val = 1;
        break;
    default:
    case 4:
        val = 2;
        break;
    case 8:
        val = 3;
        break;
    }
    resBox->setCurrentIndex(val);
    connect(resBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onCmdResampling(int)));

    //
    QHBoxLayout *hf = new QHBoxLayout();
    extraBottom()->addLayout(hf);

    QDial *knob;
    QLabel *tf;

    QVBoxLayout *vert = new QVBoxLayout();
    hf->addLayout(vert);
    QGridLayout *mat = new QGridLayout(); //3
    vert->addLayout(mat);

    mat->addWidget(new QLabel(tr("Master Gain"), this), 0, 0);
    knob = new QDial(this);
    knob->setRange(0, 100);
    //knob->setTickDelta(10);
    mat->addWidget(knob, 1, 0);
    tf = new QLabel(this);
    tf->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    tf->setAlignment(Qt::AlignCenter);
    mat->addWidget(tf, 2, 0);
    connect(knob, SIGNAL(valueChanged(int)), tf, SLOT(setNum(int)));
    connect(knob, SIGNAL(valueChanged(int)), this, SLOT(onCmdMasterGain(int)));
    knob->setValue(MPTSETTINGS.masterGain);

    mat->addWidget(new QLabel(tr("Stereo Separation"), this), 0, 1);
    knob = new QDial(this);
    knob->setRange(0, 200);
    //knob->setTickDelta(20);
    mat->addWidget(knob, 1, 1);
    tf = new QLabel(this);
    tf->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    tf->setAlignment(Qt::AlignCenter);
    mat->addWidget(tf, 2, 1);
    connect(knob, SIGNAL(valueChanged(int)), tf, SLOT(setNum(int)));
    connect(knob, SIGNAL(valueChanged(int)), this, SLOT(onCmdStereoSeparation(int)));
    knob->setValue(MPTSETTINGS.stereoSeparation);

    mat->addWidget(new QLabel(tr("Volume Ramping"), this), 0, 2);
    knob = new QDial(this);
    knob->setRange(-1, 10);
    //knob->setTickDelta(1);
    mat->addWidget(knob, 1, 2);
    tf = new QLabel(this);
    tf->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    tf->setAlignment(Qt::AlignCenter);
    mat->addWidget(tf, 2, 2);
    connect(knob, SIGNAL(valueChanged(int)), tf, SLOT(setNum(int)));
    connect(knob, SIGNAL(valueChanged(int)), this, SLOT(onCmdVolumeRamping(int)));
    knob->setValue(MPTSETTINGS.volumeRamping);
}

void openmptConfig::onCmdFrequency(int val)
{
    switch (val)
    {
    case 0:
        MPTSETTINGS.samplerate = 11025;
        break;
    case 1:
        MPTSETTINGS.samplerate = 22050;
        break;
    case 2:
        MPTSETTINGS.samplerate = 44100;
        break;
    case 3:
        MPTSETTINGS.samplerate = 48000;
        break;
    }
}

void openmptConfig::onCmdChannels(int val)
{
    MPTSETTINGS.channels = val+1;
}

void openmptConfig::onCmdResampling(int val)
{
    switch (val)
    {
    case 0:
        MPTSETTINGS.resamplingMode = 1;
        break;
    case 1:
        MPTSETTINGS.resamplingMode = 2;
        break;
    case 2:
        MPTSETTINGS.resamplingMode = 4;
        break;
    case 3:
        MPTSETTINGS.resamplingMode = 8;
        break;
    }
}

void openmptConfig::onCmdMasterGain(int val)
{
    MPTSETTINGS.masterGain = val;
}

void openmptConfig::onCmdStereoSeparation(int val)
{
    MPTSETTINGS.stereoSeparation = val;
}

void openmptConfig::onCmdVolumeRamping(int val)
{
    MPTSETTINGS.volumeRamping = val;
}
