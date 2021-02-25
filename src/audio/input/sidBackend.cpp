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

#include "sidBackend.h"

#include "settings.h"
#include "utils.h"

#include <sidplayfp/SidInfo.h>

#ifdef HAVE_SIDPLAYFP_BUILDERS_RESIDFP_H
#  include <sidplayfp/builders/residfp.h>
#endif
#ifdef HAVE_SIDPLAYFP_BUILDERS_RESID_H
#  include <sidplayfp/builders/resid.h>
#endif
#ifdef HAVE_SIDPLAYFP_BUILDERS_HARDSID_H
#  include <sidplayfp/builders/hardsid.h>
#endif
#ifdef HAVE_SIDPLAYFP_BUILDERS_EXSID_H
#  include <sidplayfp/builders/exsid.h>
#endif

#include <QCheckBox>
#include <QComboBox>
#include <QDebug>
#include <QDial>
#include <QFileDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QButtonGroup>
#include <QMessageBox>

// created by reswrap from file sid.gif
extern const unsigned char iconSid[126] =
{
    0x47,0x49,0x46,0x38,0x39,0x61,0x10,0x00,0x10,0x00,0xf2,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x86,0x00,0x00,0xff,0x86,0x00,0x00,0xff,0x00,0x00,0x86,0x82,0x86,0xcc,
    0xcc,0xcc,0x00,0x00,0x00,0x21,0xf9,0x04,0x01,0x00,0x00,0x06,0x00,0x2c,0x00,0x00,
    0x00,0x00,0x10,0x00,0x10,0x00,0x00,0x03,0x43,0x68,0xba,0xdc,0x6e,0x22,0x3e,0x16,
    0x6b,0x99,0xb0,0xc6,0xfb,0x44,0xf0,0x45,0xa8,0x09,0xcc,0x17,0x84,0x4a,0xa0,0x9e,
    0x9c,0x89,0xa6,0x00,0xbb,0xac,0xdc,0x12,0xd6,0x06,0xcd,0x10,0xbc,0xb3,0x32,0x83,
    0xe0,0xa0,0xa1,0x8a,0x2d,0x06,0x00,0xe4,0x90,0x11,0x6b,0x06,0x0c,0xb7,0xd7,0xc2,
    0x09,0xc0,0x3d,0x0a,0x80,0xac,0x75,0x72,0xc3,0x78,0x17,0x09,0x00,0x3b
};

//sid|dat
#define EXT "sid|mus|prg|p00"

// HVSC path to STIL.
#define HVSC_STIL "/DOCUMENTS/STIL.txt"

// HVSC path to BUGlist.
#define HVSC_BUGLIST "/DOCUMENTS/BUGlist.txt"

#define CREDITS "Sidplayfp<br>Copyright \u00A9 Simon White, Antti Lankila, Leandro Nini"
#define LINK    "https://github.com/libsidplayfp/libsidplayfp/"

const char sidBackend::name[] = "Sidplayfp";

const char engines[][8] =
{
#ifdef HAVE_SIDPLAYFP_BUILDERS_RESIDFP_H
    "reSIDfp",
#endif
#ifdef HAVE_SIDPLAYFP_BUILDERS_RESID_H
    "reSID",
#endif
#ifdef HAVE_SIDPLAYFP_BUILDERS_HARDSID_H
    "hardSID",
#endif
#ifdef HAVE_SIDPLAYFP_BUILDERS_EXSID_H
    "exSID",
#endif
};

const int sidAddresses[5] = { 0, 0xd420, 0xd500, 0xde00, 0xdf00 };

sidConfig_t sidBackend::_settings;

/*****************************************************************/

size_t sidBackend::fillBuffer(void* buffer, const size_t bufferSize, const unsigned int seconds)
{
    if ((_length != 0) && (seconds >= _length))
        return 0;

    return _sidplayfp->play((short*)buffer, bufferSize/sizeof(short))*2;
}

/*****************************************************************/

const char* getModelString(SidTuneInfo::model_t model)
{
    switch (model)
    {
    default:
    case SidTuneInfo::SIDMODEL_UNKNOWN:
        return "UNKNOWN";
    case SidTuneInfo::SIDMODEL_6581:
        return "6581";
    case SidTuneInfo::SIDMODEL_8580:
        return "8580";
    case SidTuneInfo::SIDMODEL_ANY:
        return "ANY";
    }
}

const char* getClockString(SidTuneInfo::clock_t clock)
{
    switch (clock)
    {
    default:
    case SidTuneInfo::CLOCK_UNKNOWN:
        return "UNKNOWN";
    case SidTuneInfo::CLOCK_PAL:
        return "PAL";
    case SidTuneInfo::CLOCK_NTSC:
        return "NTSC";
    case SidTuneInfo::CLOCK_ANY:
        return "ANY";
    }
}

/*****************************************************************/

bool sidBackend::supports(const QString& fileName)
{
    QString ext(EXT);
    ext.prepend(".*\\.(").append(")");
    qDebug() << "sidBackend::supports: " << ext;

    QRegExp rx(ext);
    return rx.exactMatch(fileName);
}

inline QStringList sidBackend::ext() const { return QString(EXT).split("|"); }

sidBackend::sidBackend() :
    inputBackend(name, iconSid, 126),
    _sidplayfp(nullptr),
    _stil(nullptr),
    _db(nullptr),
    _length(0),
    _tune(nullptr)
{
    loadSettings();

    openHvsc(_settings.hvscPath);
}

sidBackend::~sidBackend()
{
    close();

    delete _db;
    delete _stil;
}

void sidBackend::loadSettings()
{
    _settings.samplerate = load("Frequency", 44100);
    _settings.channels = load("Channels", 1);
    _settings.samplingMethod = (SidConfig::sampling_method_t)load("Sampling method", SidConfig::INTERPOLATE);
    _settings.fastSampling = load("Fast sampling", false);
    _settings.bias = load("DAC Bias", 0);
    _settings.filter6581Curve = load("Filter 6581 Curve", 500);
    _settings.filter8580Curve = load("Filter 8580 Curve", 12500);
    _settings.c64Model = (SidConfig::c64_model_t)load("C64 Model", SidConfig::PAL);
    _settings.sidModel = (SidConfig::sid_model_t)load("SID model", SidConfig::MOS6581);
    _settings.forceC64Model = load("Force C64 Model", false);
    _settings.forceSidModel = load("Force SID Model", false);
    _settings.engine = load("Engine", engines[0]);
    _settings.filter = load("Filter", true);
    _settings.hvscPath = load("HVSC", QString());
    _settings.secondSidAddress = load("Second SID address", 0);
    _settings.thirdSidAddress = load("Third SID address", 0);
    _settings.kernalPath = load("Kernal Rom", QString());
    _settings.basicPath = load("BASIC Rom", QString());
    _settings.chargenPath = load("Chargen Rom", QString());
}

void sidBackend::saveSettings()
{
    QString hvsc = load("HVSC", QString());
    if (hvsc.compare(_settings.hvscPath))
    {
        qDebug() << "Reloading HVSC from " << _settings.hvscPath;
        openHvsc(_settings.hvscPath);
    }

    save("Frequency", _settings.samplerate);
    save("Channels", _settings.channels);
    save("Sampling method", _settings.samplingMethod);
    save("Fast sampling", _settings.fastSampling);
    save("DAC Bias", _settings.bias);
    save("Filter 6581 Curve", _settings.filter6581Curve);
    save("Filter 8580 Curve", _settings.filter8580Curve);
    save("C64 Model", _settings.c64Model);
    save("SID model", _settings.sidModel);
    save("Force C64 Model", _settings.forceC64Model);
    save("Force SID Model", _settings.forceSidModel);
    save("Engine", _settings.engine);
    save("Filter", _settings.filter);
    save("HVSC", _settings.hvscPath);
    save("Second SID address", _settings.secondSidAddress);
    save("Third SID address", _settings.thirdSidAddress);
    save("Kernal Rom", _settings.kernalPath);
    save("BASIC Rom", _settings.basicPath);
    save("Chargen Rom", _settings.chargenPath);
}

const unsigned char* sidBackend::loadRom(const QString& romPath)
{
    if (romPath.isEmpty())
        return nullptr;

    QFile f;
    f.setFileName(romPath);
    if (!f.open(QIODevice::ReadOnly))
        return nullptr;

    const long size = f.size();
    char* data = new char[size];
    f.read(data, size);
    f.close();
    return (const unsigned char*)data;
}

bool sidBackend::open(const QString& fileName)
{
    close();

    _sidplayfp = new sidplayfp;

    {
        const unsigned char* kernal = loadRom(_settings.kernalPath);
        const unsigned char* basic = loadRom(_settings.basicPath);
        const unsigned char* chargen = loadRom(_settings.chargenPath);
        _sidplayfp->setRoms(kernal, basic, chargen);
        delete [] kernal;
        delete [] basic;
        delete [] chargen;
    }

    sidbuilder *emuSid = nullptr;

    int eng = 0;
#ifdef HAVE_SIDPLAYFP_BUILDERS_RESIDFP_H
    if (!_settings.engine.compare(engines[eng++]))
    {
        ReSIDfpBuilder *tmpResid = new ReSIDfpBuilder("Musiqt reSIDfp");
        tmpResid->create(_sidplayfp->info().maxsids());

        tmpResid->filter(_settings.filter);
        tmpResid->filter6581Curve((double)_settings.filter6581Curve/1000.);
        tmpResid->filter8580Curve((double)_settings.filter8580Curve);

        emuSid = (sidbuilder*)tmpResid;
    }
#endif
#ifdef HAVE_SIDPLAYFP_BUILDERS_RESID_H
    if (!_settings.engine.compare(engines[eng++]))
    {
        ReSIDBuilder *tmpResid = new ReSIDBuilder("Musiqt reSID");
        tmpResid->create(_sidplayfp->info().maxsids());

        tmpResid->filter(_settings.filter);
        tmpResid->bias((double)_settings.bias/1000.0);
        emuSid = (sidbuilder*)tmpResid;
    }
#endif
#ifdef HAVE_SIDPLAYFP_BUILDERS_HARDSID_H
    if (!_settings.engine.compare(engines[eng++]))
    {
        HardSIDBuilder *tmpHardsid = new HardSIDBuilder("Musiqt hardSID");
        tmpHardsid->create(_sidplayfp->info().maxsids());

        tmpHardsid->filter(_settings.filter);
        emuSid = (sidbuilder*)tmpHardsid;
    }
#endif
#ifdef HAVE_SIDPLAYFP_BUILDERS_EXSID_H
    if (!_settings.engine.compare(engines[eng++]))
    {
        exSIDBuilder *tmpExsid = new exSIDBuilder("Musiqt exSID");
        tmpExsid->create(_sidplayfp->info().maxsids());

        tmpExsid->filter(_settings.filter);
        emuSid = (sidbuilder*)tmpExsid;
    }
#endif

    if (emuSid == nullptr)
    {
        utils::delPtr(_sidplayfp);
        return false;
    }

    SidConfig cfg;
    cfg.defaultC64Model = _settings.c64Model;
    cfg.forceC64Model = _settings.forceC64Model;
    cfg.defaultSidModel = _settings.sidModel;
    cfg.forceSidModel = _settings.forceSidModel;
    cfg.playback = (_settings.channels == 2) ? SidConfig::STEREO : SidConfig::MONO;
    cfg.frequency = _settings.samplerate;
    cfg.secondSidAddress = _settings.secondSidAddress;
#ifdef ENABLE_3SID
    cfg.thirdSidAddress = _settings.thirdSidAddress;
#endif
    cfg.sidEmulation = emuSid;
    cfg.samplingMethod = _settings.samplingMethod;
    cfg.fastSampling = _settings.fastSampling;
    _sidplayfp->config(cfg);

    _tune = new SidTune(fileName.toUtf8().constData());
    _tune->createMD5(_md5);
    qDebug() << "Tune md5: " << _md5;

    loadTune(0);

    const SidTuneInfo* tuneInfo = _tune->getInfo();

    /*
     * SID tunes have three info strings (title, artis, released)
     * p00 files have only title
     * prg files have none
     */
    switch (tuneInfo->numberOfInfoStrings())
    {
    case 3:
        _metaData.addInfo(gettext("released"), QString::fromLatin1(tuneInfo->infoString(2)));
        // fall-through
    case 2:
        _metaData.addInfo(metaData::ARTIST, QString::fromLatin1(tuneInfo->infoString(1)));
        // fall-through
    case 1:
        _metaData.addInfo(metaData::TITLE, QString::fromLatin1(tuneInfo->infoString(0)));
    }

    /*
     * MUS files have only comments
     */
    const unsigned int n = tuneInfo->numberOfCommentStrings();
    if (n != 0)
    {
        QString info;
        for (unsigned int i=0; i<n; i++)
        {
            if (!info.isEmpty())
                info.append('\n');
            info.append(tuneInfo->commentString(i));
        }

        if (!info.isEmpty())
        {
            _metaData.addInfo(metaData::COMMENT, info);
        }
    }

    if (_stil != nullptr)
    {
        const char* fName = fileName.toUtf8().constData();
        qDebug() << "Retrieving STIL info";
        QString comment = QString(_stil->getAbsGlobalComment(fName));
        if (!comment.isEmpty())
            comment.append('\n');

        comment.append(QString(_stil->getAbsEntry(fName)));

        QString bug = QString(_stil->getAbsBug(fName));
        if (!bug.isEmpty())
        {
            comment.append('\n');
            comment.append(bug);
        }
        _metaData.addInfo(metaData::COMMENT, comment);
    }

    _metaData.addInfo(gettext("speed"), _sidplayfp->info().speedString());
    _metaData.addInfo(gettext("file format"), tuneInfo->formatString());
    _metaData.addInfo(gettext("song clock"), getClockString(tuneInfo->clockSpeed()));
#ifdef ENABLE_3SID
    _metaData.addInfo(gettext("SID model"), getModelString(tuneInfo->sidModel(0)));
    if (tuneInfo->sidChips() > 1)
    {
        _metaData.addInfo(gettext("2nd SID model"), getModelString(tuneInfo->sidModel(1)));
        _metaData.addInfo(gettext("2nd SID address"),
                          QString("$%1").arg(tuneInfo->sidChipBase(1), 4, 16, QChar('0')));
        if (tuneInfo->sidChips() > 2)
        {
            _metaData.addInfo(gettext("3rd SID model"), getModelString(tuneInfo->sidModel(2)));
            _metaData.addInfo(gettext("3rd SID address"),
                              QString("$%1").arg(tuneInfo->sidChipBase(2), 4, 16, QChar('0')));
        }
    }
#else
    _metaData.addInfo(gettext("SID model"), getModelString(tuneInfo->sidModel1()));
    if (tuneInfo->isStereo())
    {
        _metaData.addInfo(gettext("2nd SID model"), getModelString(tuneInfo->sidModel2()));
        _metaData.addInfo(gettext("2nd SID address"),
                          QString("$%1").arg(tuneInfo->sidChipBase2(), 4, 16, QChar('0')));
    }
#endif
    songLoaded(fileName);
    return true;
}

void sidBackend::close()
{
    if (_sidplayfp != nullptr)
    {
        const sidbuilder *emuSid = _sidplayfp->config().sidEmulation;
        delete emuSid;
        utils::delPtr(_sidplayfp);
    }

    utils::delPtr(_tune);

    songLoaded(QString());
}

bool sidBackend::rewind()
{
    if (_sidplayfp != nullptr)
    {
        _sidplayfp->stop();
        return true;
    }
    return false;
}

bool sidBackend::subtune(const unsigned int i)
{
    if ((_tune != nullptr) && (i <= _tune->getInfo()->songs()))
    {
        loadTune(i);
        return true;
    }
    return false;
}

void sidBackend::loadTune(const int num)
{
    _tune->selectSong(num);
    _sidplayfp->load(_tune);
    _length = _db != nullptr ? _db->length(_md5, _tune->getInfo()->currentSong()) : 0;
    time(_length);
}

void sidBackend::openHvsc(const QString& hvscPath)
{
    if (hvscPath.isEmpty())
        return;

    if (_stil == nullptr)
        _stil = new STIL(HVSC_STIL, HVSC_BUGLIST);

    if (!_stil->setBaseDir(hvscPath.toLocal8Bit().constData()))
    {
        qWarning() << _stil->getErrorStr();
        utils::delPtr(_stil);
    }

    if (_db == nullptr)
        _db = new SidDatabase();

    if (!_db->open(
            QString("%1%2DOCUMENTS%2Songlengths.txt").arg(hvscPath).arg(QDir::separator()).toUtf8().constData())
        )
    {
        qWarning() << _db->error();
        utils::delPtr(_db);
    }
}

/*****************************************************************/

#include "iconFactory.h"

#define SIDSETTINGS sidBackend::_settings

enum
{
    ID_KERNAL,
    ID_BASIC,
    ID_CHARGEN
};

sidConfig::sidConfig(QWidget* win) :
    configFrame(win, sidBackend::name, CREDITS, LINK)
{
    int val;

    matrix()->addWidget(new QLabel(tr("Frequency"), this), 0, 0);
    QComboBox *freqBox = new QComboBox(this);
    matrix()->addWidget(freqBox, 0, 1);
    {
        QStringList items;
        items << "11025" << "22050" << "44100" << "48000";
        freqBox->addItems(items);
        freqBox->setMaxVisibleItems(items.size());
    }
    switch (SIDSETTINGS.samplerate)
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
        chanBox->setMaxVisibleItems(items.size());
    }
    chanBox->setCurrentIndex(SIDSETTINGS.channels-1);
    connect(chanBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onCmdChannels(int)));

    matrix()->addWidget(new QLabel(tr("Engine"), this));
    QComboBox *engBox = new QComboBox(this);
    matrix()->addWidget(engBox);

    int eng = 0;
#ifdef HAVE_SIDPLAYFP_BUILDERS_RESIDFP_H
    engBox->addItem(engines[eng++]);
#endif
#ifdef HAVE_SIDPLAYFP_BUILDERS_RESID_H
    engBox->addItem(engines[eng++]);
#endif
#ifdef HAVE_SIDPLAYFP_BUILDERS_HARDSID_H
    engBox->addItem(engines[eng++]);
#endif
#ifdef HAVE_SIDPLAYFP_BUILDERS_EXSID_H
    engBox->addItem(engines[eng++]);
#endif

    const int numItems = engBox->count();
    engBox->setMaxVisibleItems(numItems);
    const int curItem=engBox->findData(SIDSETTINGS.engine);
    if (curItem >= 0)
        engBox->setCurrentIndex(curItem);
    connect(engBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onCmdEngine(int)));

    matrix()->addWidget(new QLabel(tr("Sampling method"), this));
    QComboBox *resBox = new QComboBox(this);
    matrix()->addWidget(resBox);
    {
        QStringList items;
        items << "Interpolate" << "Resample Interpolate";
        resBox->addItems(items);
        resBox->setMaxVisibleItems(items.size());
    }
    switch (SIDSETTINGS.samplingMethod)
    {
    default:
    case SidConfig::INTERPOLATE:
        val = 0;
        break;
    case SidConfig::RESAMPLE_INTERPOLATE:
        val = 1;
        break;
    }
    resBox->setCurrentIndex(val);
    connect(resBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onCmdSampling(int)));

    matrix()->addWidget(new QLabel(tr("C64 model"), this));
    QComboBox *clockBox = new QComboBox(this);
    matrix()->addWidget(clockBox);
    {
        QStringList items;
        items << "PAL" << "NTSC" << "OLD NTSC" << "Drean";
        clockBox->addItems(items);
        clockBox->setMaxVisibleItems(items.size());
    }
    switch (SIDSETTINGS.c64Model)
    {
    default:
    case SidConfig::PAL:
        val = 0;
        break;
    case SidConfig::NTSC:
        val = 1;
        break;
    case SidConfig::OLD_NTSC:
        val = 2;
        break;
    case SidConfig::DREAN:
        val = 3;
        break;
    }
    clockBox->setCurrentIndex(val);
    connect(clockBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onCmdClock(int)));

    matrix()->addWidget(new QLabel(tr("Force C64 model"), this));
    QCheckBox *cBox = new QCheckBox(this);
    cBox->setChecked(SIDSETTINGS.forceC64Model);
    matrix()->addWidget(cBox);
    connect(cBox, SIGNAL(toggled(bool)), this, SLOT(onCmdForceC64Model(bool)));

    matrix()->addWidget(new QLabel(tr("SID Model"), this));
    QComboBox *modelBox = new QComboBox(this);
    matrix()->addWidget(modelBox);
    {
        QStringList items;
        items << "MOS 6581" << "MOS 8580" << "Drean";
        modelBox->addItems(items);
        modelBox->setMaxVisibleItems(items.size());
    }
    switch (SIDSETTINGS.sidModel)
    {
    default:
    case SidConfig::MOS6581:
        val = 0;
        break;
    case SidConfig::MOS8580:
        val = 1;
        break;
    }
    modelBox->setCurrentIndex(val);
    connect(modelBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onCmdModel(int)));

    matrix()->addWidget(new QLabel(tr("Force SID model"), this));
    cBox = new QCheckBox(this);
    cBox->setChecked(SIDSETTINGS.forceSidModel);
    matrix()->addWidget(cBox);
    connect(cBox, SIGNAL(toggled(bool)), this, SLOT(onCmdForceSidModel(bool)));

    matrix()->addWidget(new QLabel(tr("Second SID address"), this));
    QComboBox *sidAddress = new QComboBox(this);
    matrix()->addWidget(sidAddress);
    QStringList items;
    items << "(none)" << "$D420" << "$D500" << "$DE00" << "$DF00";
    sidAddress->addItems(items);
    sidAddress->setMaxVisibleItems(5);
    val = 5;
    while (val--)
    {
        if (SIDSETTINGS.secondSidAddress == sidAddresses[val])
            break;
    }
    sidAddress->setCurrentIndex(val);
    connect(sidAddress, SIGNAL(currentIndexChanged(int)), this, SLOT(onCmdAddress2(int)));

    matrix()->addWidget(new QLabel(tr("Third SID address"), this));
    sidAddress = new QComboBox(this);
    matrix()->addWidget(sidAddress);
    sidAddress->addItems(items);
    sidAddress->setMaxVisibleItems(5);
    val = 5;
    while (val--)
    {
        if (SIDSETTINGS.thirdSidAddress == sidAddresses[val])
            break;
    }
    sidAddress->setCurrentIndex(val);
    connect(sidAddress, SIGNAL(currentIndexChanged(int)), this, SLOT(onCmdAddress3(int)));
#ifndef ENABLE_3SID
    sidAddress->setDisabled(true);
#endif

    {
        QFrame* line = new QFrame();
        line->setFrameShape(QFrame::VLine);
        line->setFrameShadow(QFrame::Sunken);
        extraLeft()->addWidget(line);
    }

    QVBoxLayout *vert = new QVBoxLayout();
    extraLeft()->addLayout(vert);
    cBox = new QCheckBox(tr("Fast sampling"));
    cBox->setChecked(SIDSETTINGS.fastSampling);
    cBox->setToolTip("Faster but inaccurate sampling");
    vert->addWidget(cBox);
    connect(cBox, SIGNAL(toggled(bool)), this, SLOT(onCmdFastSampling(bool)));
    cBox = new QCheckBox(tr("Filter"));
    cBox->setChecked(SIDSETTINGS.filter);
    cBox->setToolTip("Emulate SID filter");
    vert->addWidget(cBox);
    connect(cBox, SIGNAL(toggled(bool)), this, SLOT(onCmdFilter(bool)));

    _biasFrame = new QVBoxLayout();
    vert->addLayout(_biasFrame);
    QLabel *label = new QLabel(tr("DAC Bias for reSID"), this);
    _biasFrame->addWidget(label);
    _biasFrame->setAlignment(label, Qt::AlignHCenter);
    QDial* knob = new QDial(this);
    knob->setRange(-500, 500);
    //knob->setNotchTarget(100);
    knob->setValue(SIDSETTINGS.bias);
    _biasFrame->addWidget(knob);
    _biasFrame->setAlignment(knob, Qt::AlignHCenter);
    QLabel *tf = new QLabel(this);
    tf->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    tf->setAlignment(Qt::AlignCenter);
    tf->setNum(SIDSETTINGS.bias);
    _biasFrame->addWidget(tf);
    knob->setMaximumSize(tf->height(), tf->height());
    connect(knob, SIGNAL(valueChanged(int)), this, SLOT(setBias(int)));
    connect(knob, SIGNAL(valueChanged(int)), tf, SLOT(setNum(int)));

    _filterCurveFrame = new QVBoxLayout();
    vert->addLayout(_filterCurveFrame);
    _filterCurveFrame->addWidget(new QLabel(tr("Filter curves for reSIDfp"), this));
    QGridLayout* mat = new QGridLayout();
    _filterCurveFrame->addLayout(mat);
    mat->addWidget(new QLabel("6581", this), 0, 0, 1, 1, Qt::AlignCenter);
    mat->addWidget(new QLabel("8580", this), 0, 1, 1, 1, Qt::AlignCenter);

    knob = new QDial(this);
    knob->setRange(0, 1000);
    //knob->setTickDelta(100);
    knob->setValue(SIDSETTINGS.filter6581Curve);
    mat->addWidget(knob, 1, 0, 1, 1, Qt::AlignCenter);
    tf = new QLabel(this);
    tf->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    tf->setAlignment(Qt::AlignCenter);
    tf->setNum(SIDSETTINGS.filter6581Curve);
    mat->addWidget(tf, 2, 0);
    knob->setMaximumSize(tf->height(), tf->height());
    connect(knob, SIGNAL(valueChanged(int)), this, SLOT(setFilter6581Curve(int)));
    connect(knob, SIGNAL(valueChanged(int)), tf, SLOT(setNum(int)));

    knob = new QDial(this);
    knob->setRange(8000, 16000);
    //knob->setTickDelta(500);
    knob->setValue(SIDSETTINGS.filter8580Curve);
    mat->addWidget(knob, 1, 1, 1, 1, Qt::AlignCenter);
    tf = new QLabel(this);
    tf->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    tf->setAlignment(Qt::AlignCenter);
    tf->setNum(SIDSETTINGS.filter8580Curve);
    mat->addWidget(tf, 2, 1);
    knob->setMaximumSize(tf->height(), tf->height());
    connect(knob, SIGNAL(valueChanged(int)), this, SLOT(setFilter8580Curve(int)));
    connect(knob, SIGNAL(valueChanged(int)), tf, SLOT(setNum(int)));

    QGridLayout *frame = new QGridLayout();
    extraBottom()->addLayout(frame);

    QPushButton* button;
    QButtonGroup* group = new QButtonGroup(this);

    frame->addWidget(new QLabel(tr("HVSC path:"), this), 0, 0);
    hvscPath = new QLineEdit(this);
    hvscPath->setText(SIDSETTINGS.hvscPath);
    frame->addWidget(hvscPath, 0, 1);
    connect(hvscPath, SIGNAL(editingFinished()), this, SLOT(onCmdHvscEdited()));
    button = new QPushButton(GET_ICON(icon_documentopen), tr("&Browse"), this);
    button->setToolTip("Select HVSC directory");
    frame->addWidget(button, 0, 2);
    connect(button, SIGNAL(clicked()), this, SLOT(onCmdHvsc()));

    frame->addWidget(new QLabel(tr("Kernal Rom:"), this));
    kernalRomPath = new QLineEdit(this);
    //kernalRomPath->setMaxLength(40);
    kernalRomPath->setText(SIDSETTINGS.kernalPath);
    connect(kernalRomPath, SIGNAL(editingFinished()), this, SLOT(onCmdKernalRomEdited()));
    frame->addWidget(kernalRomPath);
    button = new QPushButton(GET_ICON(icon_documentopen), tr("&Browse"), this);
    button->setToolTip("Select Kernal Rom file");
    frame->addWidget(button);
    group->addButton(button, ID_KERNAL);

    frame->addWidget(new QLabel(tr("BASIC Rom:"), this));
    basicRomPath = new QLineEdit(this);
    //basicRomPath->setMaxLength(40);
    basicRomPath->setText(SIDSETTINGS.basicPath);
    connect(basicRomPath, SIGNAL(editingFinished()), this, SLOT(onCmdBasicRomEdited()));
    frame->addWidget(basicRomPath);
    button = new QPushButton(GET_ICON(icon_documentopen), tr("&Browse"), this);
    button->setToolTip("Select BASIC Rom file");
    frame->addWidget(button);
    group->addButton(button, ID_BASIC);

    frame->addWidget(new QLabel(tr("Chargen Rom:"), this));
    chargenRomPath = new QLineEdit(this);
    //chargenRomPath->setMaxLength(40);
    chargenRomPath->setText(SIDSETTINGS.chargenPath);
    connect(chargenRomPath, SIGNAL(editingFinished()), this, SLOT(onCmdChargenRomEdited()));
    frame->addWidget(chargenRomPath);
    button = new QPushButton(GET_ICON(icon_documentopen), tr("&Browse"), this);
    button->setToolTip("Select Chargen Rom file");
    frame->addWidget(button);
    group->addButton(button, ID_CHARGEN);

    connect(group, SIGNAL(buttonClicked(int)), this, SLOT(onCmdRom(int)));
}

void sidConfig::onCmdFrequency(int val)
{
    switch (val)
    {
    case 0:
        SIDSETTINGS.samplerate = 11025;
        break;
    case 1:
        SIDSETTINGS.samplerate = 22050;
        break;
    case 2:
        SIDSETTINGS.samplerate = 44100;
        break;
    case 3:
        SIDSETTINGS.samplerate = 48000;
        break;
    }
}

void sidConfig::onCmdChannels(int val)
{
    SIDSETTINGS.channels = val+1;
}

void sidConfig::onCmdSampling(int val)
{
    switch (val)
    {
    case 0:
        SIDSETTINGS.samplingMethod = SidConfig::INTERPOLATE;
        break;
    case 1:
        SIDSETTINGS.samplingMethod = SidConfig::RESAMPLE_INTERPOLATE;
        break;
    }
}

void sidConfig::onCmdEngine(int val)
{
    QComboBox *engBox = static_cast<QComboBox*>(sender());
    SIDSETTINGS.engine = engBox->itemText(val);

    /*if (!SIDSETTINGS.engine.compare(sidBackend::engines[0]))
    {
        _biasFrame->show();
        _filterCurveFrame->hide();
    }
    else if (!SIDSETTINGS.engine.compare(sidBackend::engines[2]))
    {
        _biasFrame->hide();
        _filterCurveFrame->show();
    }
    else
    {
        _biasFrame->hide();
        _filterCurveFrame->hide();
    }*/
}

void sidConfig::onCmdClock(int val)
{
    switch (val)
    {
    case 0:
        SIDSETTINGS.c64Model = SidConfig::PAL;
        break;
    case 1:
        SIDSETTINGS.c64Model = SidConfig::NTSC;
        break;
    case 2:
        SIDSETTINGS.c64Model = SidConfig::OLD_NTSC;
        break;
    case 3:
        SIDSETTINGS.c64Model = SidConfig::DREAN;
        break;
    }
}

void sidConfig::onCmdModel(int val)
{
    switch (val)
    {
    case 0:
        SIDSETTINGS.sidModel = SidConfig::MOS6581;
        break;
    case 1:
        SIDSETTINGS.sidModel = SidConfig::MOS8580;
        break;
    }
}

void sidConfig::onCmdAddress2(int val)
{
    SIDSETTINGS.secondSidAddress = sidAddresses[val];
}

void sidConfig::onCmdAddress3(int val)
{
    SIDSETTINGS.thirdSidAddress = sidAddresses[val];
}

void sidConfig::onCmdHvsc()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select HVSC directory"), SIDSETTINGS.hvscPath);
    if (!dir.isNull())
        SIDSETTINGS.hvscPath = dir;

    hvscPath->setText(SIDSETTINGS.hvscPath);
}

void sidConfig::onCmdRom(int val)
{
    const char* text;
    QString* romPath;
    QLineEdit* romEdit;
    switch (val)
    {
    case ID_KERNAL:
        text = "Kernal";
        romPath = &SIDSETTINGS.kernalPath;
        romEdit = kernalRomPath;
        break;
    case ID_BASIC:
        text = "BASIC";
        romPath = &SIDSETTINGS.basicPath;
        romEdit = basicRomPath;
        break;
    case ID_CHARGEN:
        text = "Chargen";
        romPath = &SIDSETTINGS.chargenPath;
        romEdit = chargenRomPath;
        break;
    default:
        return;
    }

    QString file = QFileDialog::getOpenFileName(this, QString(tr("Select %1 Rom file")).arg(text), *romPath);
    if (!file.isNull())
        *romPath = file;

    romEdit->setText(*romPath);
}

void sidConfig::onCmdForceC64Model(bool val)
{
    SIDSETTINGS.forceC64Model = val;
}

void sidConfig::onCmdForceSidModel(bool val)
{
    SIDSETTINGS.forceSidModel = val;
}

void sidConfig::onCmdFastSampling(bool val)
{
    SIDSETTINGS.fastSampling = val;
}

void sidConfig::onCmdFilter(bool val)
{
    SIDSETTINGS.filter = val;
}

void sidConfig::setBias(int val)
{
    SIDSETTINGS.bias = val;
}

void sidConfig::setFilter6581Curve(int val)
{
    SIDSETTINGS.filter6581Curve = val;
}

void sidConfig::setFilter8580Curve(int val)
{
    SIDSETTINGS.filter8580Curve = val;
}

bool sidConfig::checkPath(const QString& path)
{
    if (!path.isEmpty() && !QFileInfo(path).exists())
    {
        QMessageBox::warning(this, tr("Warning"), tr("Path does not exists"));
        return false;
    }
    return true;
}

void sidConfig::onCmdHvscEdited()
{
    QString val = hvscPath->text();
    if (checkPath(val))
    {
        SIDSETTINGS.hvscPath = val;
    }
}

void sidConfig::onCmdKernalRomEdited()
{
    QString val = kernalRomPath->text();
    if (checkPath(val))
    {
        SIDSETTINGS.kernalPath = val;
    }
}

void sidConfig::onCmdBasicRomEdited()
{
    QString val = basicRomPath->text();
    if (checkPath(val))
    {
        SIDSETTINGS.basicPath = val;
    }
}

void sidConfig::onCmdChargenRomEdited()
{
    QString val = chargenRomPath->text();
    if (checkPath(val))
    {
        SIDSETTINGS.chargenPath = val;
    }
}
