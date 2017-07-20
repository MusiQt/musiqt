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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "gmeBackend.h"

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

#ifdef HAVE_LIBZ
#  define EXT "ay|gbs|gym|hes|kss|nsfe|nsf|sap|spc|vgm|vgz"
#else
#  define EXT "ay|gbs|gym|hes|kss|nsfe|nsf|sap|spc|vgm"
#endif

// ASMA path to STIL.
#define ASMA_STIL "/Docs/STIL.txt"

// ASMA path to BUGlist.
#define ASMA_BUGLIST "/Docs/Bugs.txt"

#define CREDITS "Game_Music_Emu library 0.6.1\nCopyright \302\251 Shay Green, Michael Pyne."
#define LINK    "https://bitbucket.org/mpyne/game-music-emu/"

const char gmeBackend::name[] = "Gme";

gmeConfig_t gmeBackend::_settings;

/*****************************************************************/

size_t gmeBackend::fillBuffer(void* buffer, const size_t bufferSize, const unsigned int seconds)
{
    if (_emu->track_ended())
        return 0;

    _emu->play((bufferSize>>1), (Music_Emu::sample_t*)buffer);
    return bufferSize;
}

/*****************************************************************/

bool gmeBackend::supports(const QString& fileName)
{
    QString ext(EXT);
    ext.prepend(".*\\.(").append(")");
    qDebug() << "sidBackend::supports: " << ext;

    QRegExp rx(ext);
    return rx.exactMatch(fileName);
}

inline QStringList gmeBackend::ext() const { return QString(EXT).split("|"); }

gmeBackend::gmeBackend() :
    inputBackend(name),
    _emu(nullptr),
    _stil(nullptr)
{
    loadSettings();
    openAsma(_settings.asmaPath);
}

gmeBackend::~gmeBackend()
{
    close();

    delete _stil;
}

void gmeBackend::loadSettings()
{
    _settings.samplerate = load("Samplerate", 44100);
    _settings.equalizer = load("Equalizer", false);
    _settings.treble_dB = load("Treble dB", 0.0);
    _settings.bass_freq = load("Bass freq", 15);
    _settings.asmaPath = load("ASMA", QString::null);
}

void gmeBackend::saveSettings()
{
    if (_settings.asmaPath.compare(load("ASMA", QString::null)))
    {
        qDebug() << "Reloading ASMA from " << _settings.asmaPath;
        openAsma(_settings.asmaPath);
    }

    save("Samplerate", _settings.samplerate);
    save("Equalizer", _settings.equalizer);
    save("Treble dB", _settings.treble_dB);
    save("Bass freq", _settings.bass_freq);
    save("ASMA", _settings.asmaPath);
}

bool gmeBackend::open(const QString& fileName)
{
    close();

    gme_type_t fileType;
    if (!check(gme_identify_file(fileName.toUtf8().constData(), &fileType)))
        return false;

    qDebug() << "System " << fileType->system;

    _emu = fileType->new_emu();
    if (!check(_emu->set_sample_rate(_settings.samplerate)))
        return false;
    if (_settings.equalizer)
    {
        gme_equalizer_t eq = { _settings.treble_dB, _settings.bass_freq };
        gme_set_equalizer(_emu, &eq);
    }
    if (!check(_emu->load_file(fileName.toUtf8().constData())))
        return false;
    if (!check(_emu->start_track(0)))
        return false;

    QFileInfo fInfo(fileName);
    _emu->load_m3u(QString("%1%2.m3u").arg(fInfo.canonicalPath()).arg(fInfo.completeBaseName()).toLocal8Bit().constData());

    getInfo();

    _hasStilInfo = _stil && !fInfo.suffix().compare("sap", Qt::CaseInsensitive);
    if (_hasStilInfo)
    {
        qDebug("Retrieving STIL info");
        QString comment = QString::fromLatin1(_stil->getAbsGlobalComment(fileName.toLocal8Bit().constData()));
        if (!comment.isEmpty())
            comment.append('\n');
        comment.append(QString::fromLatin1(_stil->getAbsEntry(fileName.toLocal8Bit().constData())));
        QString bug = QString::fromLatin1(_stil->getAbsBug(fileName.toLocal8Bit().constData()));
        if (!bug.isEmpty())
        {
            comment.append('\n');
            comment.append(bug);
        }
        if (!comment.isEmpty())
        {
            _metaData.addInfo(metaData::COMMENT, comment);
        }
    }

    songLoaded(fileName);
    return true;
}

void gmeBackend::getInfo()
{
    track_info_t ti;
    const char* err = _emu->track_info(&ti);
    if (err)
    {
        qWarning() << "Warning: " << err;
        return;
    }

    _metaData.addInfo(metaData::TITLE, ti.song);
    _metaData.addInfo(metaData::ARTIST, ti.author);
    _metaData.addInfo(gettext("copyright"), ti.copyright);
    _metaData.addInfo(gettext("system"), ti.system);
    _metaData.addInfo(gettext("game"), ti.game);
    _metaData.addInfo(gettext("dumper"), ti.dumper);
    if (!_hasStilInfo)
        _metaData.addInfo(metaData::COMMENT, ti.comment);

    time(ti.length/1000);
}

bool gmeBackend::check(const char* error)
{
    if (error)
    {
        utils::delPtr(_emu);
        qWarning() << "Error: " << error;
        return false;
    }
    return true;
}

void gmeBackend::close()
{
    utils::delPtr(_emu);

    songLoaded(QString::null);
}

bool gmeBackend::rewind()
{
    if (_emu != nullptr)
    {
        _emu->start_track(_emu->current_track());
        return true;
    }
    return false;
}

bool gmeBackend::subtune(const unsigned int i)
{
    if ((_emu != nullptr) && (i > 0) && (i <= _emu->track_count()))
    {
        _emu->start_track(i-1);
        getInfo();
        return true;
    }
    return false;
}

void gmeBackend::openAsma(const QString& asmaPath)
{
    if (asmaPath.isEmpty())
        return;

    if (_stil == nullptr)
        _stil = new STIL(ASMA_STIL, ASMA_BUGLIST);

    if (!_stil->setBaseDir(asmaPath.toLocal8Bit().constData()))
    {
        qWarning() << _stil->getErrorStr();
        utils::delPtr(_stil);
    }
}

/*****************************************************************/

#include "iconFactory.h"

#define GMESETTINGS gmeBackend::_settings

gmeConfig::gmeConfig(QWidget* win) :
    configFrame(win, gmeBackend::name, CREDITS, LINK)
{
    int val;

    matrix()->addWidget(new QLabel(tr("Samplerate"), this), 0, 0);
    QComboBox *freqBox = new QComboBox(this);
    matrix()->addWidget(freqBox, 0, 1);
    {
        QStringList items;
        items << "11025" << "22050" << "44100" << "48000";
        freqBox->addItems(items);
        freqBox->setMaxVisibleItems(items.size());
    }
    switch (GMESETTINGS.samplerate)
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

    {
        QVBoxLayout *equalizerBox = new QVBoxLayout();
        QGroupBox *group = new QGroupBox(tr("Equalizer"));
        group->setCheckable(true);
        group->setToolTip(tr("Enable equalizer"));
        group->setChecked(GMESETTINGS.equalizer);
        group->setLayout(equalizerBox);
        connect(group, SIGNAL(toggled(bool)), this, SLOT(setEqualizer(bool)));

        QGridLayout* mat = new QGridLayout();
        equalizerBox->addLayout(mat);
        mat->addWidget(new QLabel("Treble DB", this), 0, 0, 1, 1, Qt::AlignCenter);
        mat->addWidget(new QLabel("Bass frequency", this), 0, 1, 1, 1, Qt::AlignCenter);

        QDial *knob = new QDial(this);
        knob->setRange(-50, 5);
        //knob->setTickDelta(100);
        knob->setValue(GMESETTINGS.treble_dB);
        mat->addWidget(knob, 1, 0, 1, 1, Qt::AlignCenter);
        QLabel *tf = new QLabel(this);
        tf->setFrameStyle(QFrame::Panel|QFrame::Sunken);
        tf->setAlignment(Qt::AlignCenter);
        tf->setNum(GMESETTINGS.treble_dB);
        mat->addWidget(tf, 2, 0);
        knob->setMaximumSize(tf->height(), tf->height());
        connect(knob, SIGNAL(valueChanged(int)), this, SLOT(setTrebledB(int)));
        connect(knob, SIGNAL(valueChanged(int)), tf, SLOT(setNum(int)));

        knob = new QDial(this);
        knob->setRange(1, 16000);
        //knob->setTickDelta(500);
        knob->setValue(GMESETTINGS.bass_freq);
        mat->addWidget(knob, 1, 1, 1, 1, Qt::AlignCenter);
        tf = new QLabel(this);
        tf->setFrameStyle(QFrame::Panel|QFrame::Sunken);
        tf->setAlignment(Qt::AlignCenter);
        tf->setNum(GMESETTINGS.bass_freq);
        mat->addWidget(tf, 2, 1);
        knob->setMaximumSize(tf->height(), tf->height());
        connect(knob, SIGNAL(valueChanged(int)), this, SLOT(setBassFreq(int)));
        connect(knob, SIGNAL(valueChanged(int)), tf, SLOT(setNum(int)));

        matrix()->addWidget(group);
    }

    QHBoxLayout *hf = new QHBoxLayout();
    extraBottom()->addLayout(hf);

    hf->addWidget(new QLabel(tr("ASMA path:"), this));
    asmaPath = new QLineEdit(this);
    asmaPath->setText(GMESETTINGS.asmaPath);
    hf->addWidget(asmaPath);
    connect(asmaPath, SIGNAL(editingFinished()), this, SLOT(onCmdAsmaEdited()));
    QPushButton* button = new QPushButton(GET_ICON(icon_documentopen), tr("&Browse"), this);
    button->setToolTip(tr("Select ASMA directory"));
    hf->addWidget(button);
    connect(button, SIGNAL(clicked()), this, SLOT(onCmdAsma()));
}

void gmeConfig::onCmdSamplerate(int val)
{
    switch (val)
    {
    case 0:
        GMESETTINGS.samplerate = 11025;
        break;
    case 1:
        GMESETTINGS.samplerate = 22050;
        break;
    case 2:
        GMESETTINGS.samplerate = 44100;
        break;
    case 3:
        GMESETTINGS.samplerate = 48000;
        break;
    }
}

void gmeConfig::setEqualizer(bool val)
{
    GMESETTINGS.equalizer = val;
}

void gmeConfig::setTrebledB(int val)
{
    GMESETTINGS.treble_dB = val;
}

void gmeConfig::setBassFreq(int val)
{
    GMESETTINGS.bass_freq = val;
}

void gmeConfig::onCmdAsma()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select ASMA directory"), GMESETTINGS.asmaPath);
    if (!dir.isNull())
        GMESETTINGS.asmaPath = dir;

    asmaPath->setText(GMESETTINGS.asmaPath);
}

void gmeConfig::onCmdAsmaEdited()
{
    QString val = asmaPath->text();
    if (!QFileInfo(val).exists())
    {
        QMessageBox::warning(this, tr("Warning"), tr("Path does not exists"));
    }
    else
    {
        GMESETTINGS.asmaPath = val;
    }
}
