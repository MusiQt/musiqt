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
#include <QFile>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QButtonGroup>
#include <QMessageBox>

#include <memory>

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

sidConfig_t sidConfig::m_settings;

inputConfig* sidBackend::cFactory() { return new sidConfig(name, iconSid, 126); }

/*****************************************************************/

size_t sidBackend::fillBuffer(void* buffer, const size_t bufferSize)
{
    return m_sidplayfp->play((short*)buffer, bufferSize/sizeof(short))*2;
}

/*****************************************************************/

void sidConfig::loadSettings()
{
    m_settings.samplerate = load("Frequency", 44100);
    m_settings.channels = load("Channels", 1);
    m_settings.samplingMethod = (SidConfig::sampling_method_t)load("Sampling method", SidConfig::INTERPOLATE);
    m_settings.fastSampling = load("Fast sampling", false);
    m_settings.bias = load("DAC Bias", 0);
    m_settings.filter6581Curve = load("Filter 6581 Curve", 500);
    m_settings.filter8580Curve = load("Filter 8580 Curve", 12500);
    m_settings.c64Model = (SidConfig::c64_model_t)load("C64 Model", SidConfig::PAL);
    m_settings.sidModel = (SidConfig::sid_model_t)load("SID model", SidConfig::MOS6581);
    m_settings.forceC64Model = load("Force C64 Model", false);
    m_settings.forceSidModel = load("Force SID Model", false);
    m_settings.engine = load("Engine", engines[0]);
    m_settings.filter = load("Filter", true);
    m_settings.hvscPath = load("HVSC", QString());
    m_settings.secondSidAddress = load("Second SID address", 0);
    m_settings.thirdSidAddress = load("Third SID address", 0);
    m_settings.kernalPath = load("Kernal Rom", QString());
    m_settings.basicPath = load("BASIC Rom", QString());
    m_settings.chargenPath = load("Chargen Rom", QString());
}

void sidConfig::saveSettings()
{
    save("Frequency", m_settings.samplerate);
    save("Channels", m_settings.channels);
    save("Sampling method", m_settings.samplingMethod);
    save("Fast sampling", m_settings.fastSampling);
    save("DAC Bias", m_settings.bias);
    save("Filter 6581 Curve", m_settings.filter6581Curve);
    save("Filter 8580 Curve", m_settings.filter8580Curve);
    save("C64 Model", m_settings.c64Model);
    save("SID model", m_settings.sidModel);
    save("Force C64 Model", m_settings.forceC64Model);
    save("Force SID Model", m_settings.forceSidModel);
    save("Engine", m_settings.engine);
    save("Filter", m_settings.filter);
    save("HVSC", m_settings.hvscPath);
    save("Second SID address", m_settings.secondSidAddress);
    save("Third SID address", m_settings.thirdSidAddress);
    save("Kernal Rom", m_settings.kernalPath);
    save("BASIC Rom", m_settings.basicPath);
    save("Chargen Rom", m_settings.chargenPath);
}

/*****************************************************************/

static inline unsigned char petscii2ascii(unsigned char ch)
{
   if ((ch > 0xc0) && (ch < 0xdb)) return ch - 96;                         // lowercase chars
   else if ((ch >= 0x40) && (ch < 0x5e)) return (ch != 0x5c) ? ch : 0xa3;  // uppercase chars
   else if (ch == 0xa4) return 0x5f;                                       // underscore
   else if ((ch >= 0x00) && (ch < 0x20)) return (ch == 0x0d) ? ch : 0;     // control codes
   else if ((ch >= 0x80) && (ch < 0xa0)) return (ch == 0x8d) ? 0x0a : 0;
   return 0x20;
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

QStringList sidBackend::ext() { return QString(EXT).split("|"); }

sidBackend::sidBackend(const QString& fileName) :
    m_stil(nullptr),
    m_db(nullptr),
    m_newSonglengthDB(false),
    m_config(name, iconSid, 126)
{
    createEmu();

    std::unique_ptr<SidTune> sidTune(new SidTune(fileName.toUtf8().constData()));
    if (!sidTune->getStatus())
    {
        QString error(sidTune->statusString());
        deleteEmu();
        throw loadError(error);
    }

    openHvsc(m_config.hvscPath());

    if (fileName.endsWith(".mus"))
    {
        loadWDS(fileName, "wds");
    }
    else if (fileName.endsWith(".MUS"))
    {
        loadWDS(fileName, "WDS");
    }

    m_tune = sidTune.release();
    loadTune(0);

    getInfo(m_tune->getInfo());

    if (m_stil != nullptr)
    {
        const char* fName = fileName.toUtf8().constData();
        qDebug() << "Retrieving STIL info";
        QString comment = QString(m_stil->getAbsGlobalComment(fName));
        if (!comment.isEmpty())
            comment.append('\n');

        comment.append(QString(m_stil->getAbsEntry(fName)));

        QString bug = QString(m_stil->getAbsBug(fName));
        if (!bug.isEmpty())
        {
            comment.append('\n');
            comment.append(bug);
        }
        m_metaData.addInfo(metaData::COMMENT, comment);
    }

    songLoaded(fileName);
}

sidBackend::~sidBackend()
{
    deleteEmu();

    delete m_tune;

    delete m_db;
    delete m_stil;
}

void sidBackend::deleteEmu()
{
    sidbuilder *emuSid = m_sidplayfp->config().sidEmulation;
    delete m_sidplayfp;
    delete emuSid;
}

void sidBackend::createEmu()
{
    std::unique_ptr<sidplayfp> emu(new sidplayfp());

    {
        const unsigned char* kernal = loadRom(m_config.kernalPath());
        const unsigned char* basic = loadRom(m_config.basicPath());
        const unsigned char* chargen = loadRom(m_config.chargenPath());
        emu->setRoms(kernal, basic, chargen);
        delete [] kernal;
        delete [] basic;
        delete [] chargen;
    }

    sidbuilder *emuSid = nullptr;

    int eng = 0;
#ifdef HAVE_SIDPLAYFP_BUILDERS_RESIDFP_H
    if (!m_config.engine().compare(engines[eng++]))
    {
        ReSIDfpBuilder *tmpResid = new ReSIDfpBuilder("Musiqt reSIDfp");
        tmpResid->create(emu->info().maxsids());

        tmpResid->filter(m_config.filter());
        tmpResid->filter6581Curve((double)m_config.filter6581Curve()/1000.);
        tmpResid->filter8580Curve((double)m_config.filter8580Curve());

        emuSid = (sidbuilder*)tmpResid;
    }
#endif
#ifdef HAVE_SIDPLAYFP_BUILDERS_RESID_H
    if (!m_config.engine().compare(engines[eng++]))
    {
        ReSIDBuilder *tmpResid = new ReSIDBuilder("Musiqt reSID");
        tmpResid->create(emu->info().maxsids());

        tmpResid->filter(m_config.filter());
        tmpResid->bias((double)m_config.bias()/1000.0);

        emuSid = (sidbuilder*)tmpResid;
    }
#endif
#ifdef HAVE_SIDPLAYFP_BUILDERS_HARDSID_H
    if (!m_config.engine().compare(engines[eng++]))
    {
        HardSIDBuilder *tmpHardsid = new HardSIDBuilder("Musiqt hardSID");
        tmpHardsid->create(emu->info().maxsids());

        tmpHardsid->filter(m_config.filter());

        emuSid = (sidbuilder*)tmpHardsid;
    }
#endif
#ifdef HAVE_SIDPLAYFP_BUILDERS_EXSID_H
    if (!m_config.engine().compare(engines[eng++]))
    {
        exSIDBuilder *tmpExsid = new exSIDBuilder("Musiqt exSID");
        tmpExsid->create(emu->info().maxsids());

        tmpExsid->filter(m_config.filter());

        emuSid = (sidbuilder*)tmpExsid;
    }
#endif

    if (emuSid == nullptr)
    {
        throw loadError(QString("Error creating emu engine %1").arg(m_config.engine()));
    }

    SidConfig cfg;
    cfg.defaultC64Model = m_config.c64Model();
    cfg.forceC64Model = m_config.forceC64Model();
    cfg.defaultSidModel = m_config.sidModel();
    cfg.forceSidModel = m_config.forceSidModel();
    cfg.playback = (m_config.channels() == 2) ? SidConfig::STEREO : SidConfig::MONO;
    cfg.frequency = m_config.samplerate();
    cfg.secondSidAddress = m_config.secondSidAddress();
#ifdef ENABLE_3SID
    cfg.thirdSidAddress = m_config.thirdSidAddress();
#endif
    cfg.sidEmulation = emuSid;
    cfg.samplingMethod = m_config.samplingMethod();
    cfg.fastSampling = m_config.fastSampling();
    emu->config(cfg);

    m_sidplayfp = emu.release();
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

bool sidBackend::rewind()
{
    m_sidplayfp->stop();
    return true;
}

bool sidBackend::subtune(const unsigned int i)
{
    if (i <= m_tune->getInfo()->songs())
    {
        loadTune(i);
        return true;
    }
    return false;
}

void sidBackend::loadTune(const int num)
{
    m_tune->selectSong(num);
    m_sidplayfp->load(m_tune);
    if (m_db != nullptr)
    {
        int_least32_t songLength;
#if LIBSIDPLAYFP_VERSION_MAJ > 1
        songLength = m_newSonglengthDB ? m_db->lengthMs(*m_tune) : (m_db->length(*m_tune) * 1000);
#else
        char md5[SidTune::MD5_LENGTH+1];
        m_tune->createMD5(md5);
        qDebug() << "Tune md5: " << md5;
        songLength = m_db->length(md5, m_tune->getInfo()->currentSong()) * 1000;
#endif
        setDuration((songLength < 0) ? 0 : songLength);
    }
}

void sidBackend::getInfo(const SidTuneInfo* tuneInfo) noexcept
{
    /*
     * SID tunes have three info strings (title, artis, released)
     * p00 files have only title
     * prg files have none
     */
    switch (tuneInfo->numberOfInfoStrings())
    {
    case 3:
        m_metaData.addInfo(gettext("released"), QString::fromLatin1(tuneInfo->infoString(2)));
        // fall-through
    case 2:
        m_metaData.addInfo(metaData::ARTIST, QString::fromLatin1(tuneInfo->infoString(1)));
        // fall-through
    case 1:
        m_metaData.addInfo(metaData::TITLE, QString::fromLatin1(tuneInfo->infoString(0)));
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
            m_metaData.addInfo(metaData::COMMENT, info);
        }
    }

    m_metaData.addInfo(gettext("speed"), m_sidplayfp->info().speedString());
    m_metaData.addInfo(gettext("file format"), tuneInfo->formatString());
    m_metaData.addInfo(gettext("song clock"), getClockString(tuneInfo->clockSpeed()));
#ifdef ENABLE_3SID
    m_metaData.addInfo(gettext("SID model"), getModelString(tuneInfo->sidModel(0)));
    if (tuneInfo->sidChips() > 1)
    {
        m_metaData.addInfo(gettext("2nd SID model"), getModelString(tuneInfo->sidModel(1)));
        m_metaData.addInfo(gettext("2nd SID address"),
                          QString("$%1").arg(tuneInfo->sidChipBase(1), 4, 16, QChar('0')));
        if (tuneInfo->sidChips() > 2)
        {
            m_metaData.addInfo(gettext("3rd SID model"), getModelString(tuneInfo->sidModel(2)));
            m_metaData.addInfo(gettext("3rd SID address"),
                              QString("$%1").arg(tuneInfo->sidChipBase(2), 4, 16, QChar('0')));
        }
    }
#else
    m_metaData.addInfo(gettext("SID model"), getModelString(tuneInfo->sidModel1()));
    if (tuneInfo->isStereo())
    {
        m_metaData.addInfo(gettext("2nd SID model"), getModelString(tuneInfo->sidModel2()));
        m_metaData.addInfo(gettext("2nd SID address"),
                          QString("$%1").arg(tuneInfo->sidChipBase2(), 4, 16, QChar('0')));
    }
#endif
}

void sidBackend::openHvsc(const QString& hvscPath)
{
    if (hvscPath.isEmpty())
        return;

    m_stil = new STIL(HVSC_STIL, HVSC_BUGLIST);

    if (!m_stil->setBaseDir(hvscPath.toLocal8Bit().constData()))
    {
        qWarning() << m_stil->getErrorStr();
        utils::delPtr(m_stil);
    }

    m_db = new SidDatabase();

    QString slDbPath(QString("%1%2DOCUMENTS%2Songlengths").arg(hvscPath, QDir::separator()));
    qDebug() << "SL DB path: " << slDbPath;
    if (m_db->open(QString(slDbPath).append(".md5").toUtf8().constData()))
    {
        m_newSonglengthDB = true;
    }
    else if (!m_db->open(QString(slDbPath).append(".txt").toUtf8().constData()))
    {
        qWarning() << m_db->error();
        utils::delPtr(m_db);
    }
}

void sidBackend::loadWDS(const QString& musFileName, const char* ext)
{
    QString wdsFileName(musFileName);
    wdsFileName.chop(3);
    wdsFileName.append(ext);
    if (QFile::exists(wdsFileName))
    {
        QFile wdsFile(wdsFileName);
        if (wdsFile.open(QIODevice::ReadOnly))
        {
            QByteArray petscii = wdsFile.readAll();
            QByteArray ascii;
            for (auto ch : petscii)
            {
                unsigned char val = petscii2ascii(ch);
                if (val)
                    ascii.push_back(val);
            }

            m_metaData.addInfo(metaData::LYRICS, QString::fromLatin1(ascii));
        }
    }
}

/*****************************************************************/

#include "iconFactory.h"

#define SIDSETTINGS sidConfig::m_settings

enum
{
    ID_KERNAL,
    ID_BASIC,
    ID_CHARGEN
};

sidConfigFrame::sidConfigFrame(QWidget* win) :
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
    connect(freqBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
        [](int val) {
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
    );

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
    connect(chanBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
        [](int val) {
            SIDSETTINGS.channels = val+1;
        }
    );

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
    connect(engBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
        [engBox](int val) {
            SIDSETTINGS.engine = engBox->itemText(val);
        }
    );

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
    connect(resBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
        [](int val) {
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
    );

    matrix()->addWidget(new QLabel(tr("Default C64 model"), this));
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
    connect(clockBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
        [](int val) {
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
    );

    matrix()->addWidget(new QLabel(tr("Force C64 model"), this));
    QCheckBox *cBox = new QCheckBox(this);
    cBox->setChecked(SIDSETTINGS.forceC64Model);
    matrix()->addWidget(cBox);
    connect(cBox, &QCheckBox::toggled,
        [](bool val) {
            SIDSETTINGS.forceC64Model = val;
        }
    );

    matrix()->addWidget(new QLabel(tr("Default SID model"), this));
    QComboBox *modelBox = new QComboBox(this);
    matrix()->addWidget(modelBox);
    {
        QStringList items;
        items << "MOS 6581" << "MOS 8580";
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
    connect(modelBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
        [](int val) {
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
    );

    matrix()->addWidget(new QLabel(tr("Force SID model"), this));
    cBox = new QCheckBox(this);
    cBox->setChecked(SIDSETTINGS.forceSidModel);
    matrix()->addWidget(cBox);
    connect(cBox, &QCheckBox::toggled,
        [](bool val) {
            SIDSETTINGS.forceSidModel = val;
        }
    );

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
    connect(sidAddress, QOverload<int>::of(&QComboBox::currentIndexChanged),
        [](int val) {
            SIDSETTINGS.secondSidAddress = sidAddresses[val];
        }
    );

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
    connect(sidAddress, QOverload<int>::of(&QComboBox::currentIndexChanged),
        [](int val) {
            SIDSETTINGS.thirdSidAddress = sidAddresses[val];
        }
    );
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
    connect(cBox, &QCheckBox::toggled,
        [](bool val) {
            SIDSETTINGS.fastSampling = val;
        }
    );
    cBox = new QCheckBox(tr("Filter"));
    cBox->setChecked(SIDSETTINGS.filter);
    cBox->setToolTip("Emulate SID filter");
    vert->addWidget(cBox);
    connect(cBox, &QCheckBox::toggled,
        [](bool val) {
            SIDSETTINGS.filter = val;
        }
    );

    QVBoxLayout *biasFrame = new QVBoxLayout();
    vert->addLayout(biasFrame);
    QLabel *label = new QLabel(tr("DAC Bias for reSID"), this);
    biasFrame->addWidget(label);
    biasFrame->setAlignment(label, Qt::AlignHCenter);
    QDial* knob = new QDial(this);
    knob->setRange(-500, 500);
    knob->setValue(SIDSETTINGS.bias);
    biasFrame->addWidget(knob);
    biasFrame->setAlignment(knob, Qt::AlignHCenter);
    QLabel *tf = new QLabel(this);
    tf->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    tf->setAlignment(Qt::AlignCenter);
    tf->setNum(SIDSETTINGS.bias);
    biasFrame->addWidget(tf);
    knob->setMaximumSize(tf->height(), tf->height());
    connect(knob, &QDial::valueChanged,
        [tf](int val) {
            SIDSETTINGS.bias = val;
            tf->setNum(val);
        }
    );

    QVBoxLayout *filterCurveFrame = new QVBoxLayout();
    vert->addLayout(filterCurveFrame);
    filterCurveFrame->addWidget(new QLabel(tr("Filter curves for reSIDfp"), this));
    QGridLayout* mat = new QGridLayout();
    filterCurveFrame->addLayout(mat);
    mat->addWidget(new QLabel("6581", this), 0, 0, 1, 1, Qt::AlignCenter);
    mat->addWidget(new QLabel("8580", this), 0, 1, 1, 1, Qt::AlignCenter);

    knob = new QDial(this);
    knob->setRange(0, 1000);
    knob->setValue(SIDSETTINGS.filter6581Curve);
    mat->addWidget(knob, 1, 0, 1, 1, Qt::AlignCenter);
    tf = new QLabel(this);
    tf->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    tf->setAlignment(Qt::AlignCenter);
    tf->setNum(SIDSETTINGS.filter6581Curve);
    mat->addWidget(tf, 2, 0);
    knob->setMaximumSize(tf->height(), tf->height());
    connect(knob, &QDial::valueChanged,
        [tf](int val) {
            SIDSETTINGS.filter6581Curve = val;
            tf->setNum(val);
        }
    );

    knob = new QDial(this);
    knob->setRange(8000, 16000);
    knob->setValue(SIDSETTINGS.filter8580Curve);
    mat->addWidget(knob, 1, 1, 1, 1, Qt::AlignCenter);
    tf = new QLabel(this);
    tf->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    tf->setAlignment(Qt::AlignCenter);
    tf->setNum(SIDSETTINGS.filter8580Curve);
    mat->addWidget(tf, 2, 1);
    knob->setMaximumSize(tf->height(), tf->height());
    connect(knob, &QDial::valueChanged,
        [tf](int val) {
            SIDSETTINGS.filter8580Curve = val;
            tf->setNum(val);
        }
    );

    QGridLayout *frame = new QGridLayout();
    extraBottom()->addLayout(frame);

    QPushButton* button;
    QButtonGroup* group = new QButtonGroup(this);

    frame->addWidget(new QLabel(tr("HVSC path:"), this), 0, 0);
    QLineEdit *hvscPath = new QLineEdit(this);
    hvscPath->setText(SIDSETTINGS.hvscPath);
    frame->addWidget(hvscPath, 0, 1);
    connect(hvscPath, &QLineEdit::editingFinished,
        [hvscPath, this]() {
            QString val = hvscPath->text();
            if (checkPath(val))
            {
                SIDSETTINGS.hvscPath = val;
            }
        }
    );
    button = new QPushButton(GET_ICON(icon_documentopen), tr("&Browse"), this);
    button->setToolTip("Select HVSC directory");
    frame->addWidget(button, 0, 2);
    connect(button, &QPushButton::clicked,
        [hvscPath, this]() {
            QString dir = QFileDialog::getExistingDirectory(this, tr("Select HVSC directory"), SIDSETTINGS.hvscPath);
            if (!dir.isNull())
                SIDSETTINGS.hvscPath = dir;

            hvscPath->setText(SIDSETTINGS.hvscPath);
        }
    );

    frame->addWidget(new QLabel(tr("Kernal Rom:"), this));
    QLineEdit *kernalRomPath = new QLineEdit(this);
    kernalRomPath->setText(SIDSETTINGS.kernalPath);
    connect(kernalRomPath, &QLineEdit::editingFinished,
        [kernalRomPath, this]() {
            QString val = kernalRomPath->text();
            if (checkPath(val))
            {
                SIDSETTINGS.kernalPath = val;
            }
        }
    );
    frame->addWidget(kernalRomPath);
    button = new QPushButton(GET_ICON(icon_documentopen), tr("&Browse"), this);
    button->setToolTip("Select Kernal Rom file");
    frame->addWidget(button);
    group->addButton(button, ID_KERNAL);

    frame->addWidget(new QLabel(tr("BASIC Rom:"), this));
    QLineEdit *basicRomPath = new QLineEdit(this);
    basicRomPath->setText(SIDSETTINGS.basicPath);
    connect(basicRomPath, &QLineEdit::editingFinished,
        [kernalRomPath, this]() {
            QString val = kernalRomPath->text();
            if (checkPath(val))
            {
                SIDSETTINGS.basicPath = val;
            }
        }
    );
    frame->addWidget(basicRomPath);
    button = new QPushButton(GET_ICON(icon_documentopen), tr("&Browse"), this);
    button->setToolTip("Select BASIC Rom file");
    frame->addWidget(button);
    group->addButton(button, ID_BASIC);

    frame->addWidget(new QLabel(tr("Chargen Rom:"), this));
    QLineEdit *chargenRomPath = new QLineEdit(this);
    chargenRomPath->setText(SIDSETTINGS.chargenPath);
    connect(chargenRomPath, &QLineEdit::editingFinished,
        [kernalRomPath, this]() {
            QString val = kernalRomPath->text();
            if (checkPath(val))
            {
                SIDSETTINGS.chargenPath = val;
            }
        }
    );
    frame->addWidget(chargenRomPath);
    button = new QPushButton(GET_ICON(icon_documentopen), tr("&Browse"), this);
    button->setToolTip("Select Chargen Rom file");
    frame->addWidget(button);
    group->addButton(button, ID_CHARGEN);

    connect(group, &QButtonGroup::idClicked,
        [kernalRomPath, basicRomPath, chargenRomPath, this](int val) {
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
    );
}

bool sidConfigFrame::checkPath(const QString& path)
{
    if (!path.isEmpty() && !QFileInfo(path).exists())
    {
        QMessageBox::warning(this, tr("Warning"), tr("Path does not exists"));
        return false;
    }
    return true;
}
