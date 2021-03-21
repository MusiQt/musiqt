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

#include <string>

#define EXT "ay|gbs|gym|hes|kss|nsfe|nsf|sap|spc|vgm|vgz"

// ASMA path to STIL.
#define ASMA_STIL "/Docs/STIL.txt"

// ASMA path to BUGlist.
#define ASMA_BUGLIST "/Docs/Bugs.txt"

#define CREDITS "Game_Music_Emu library<br>Copyright \302\251 Shay Green, Michael Pyne."
#define LINK    "https://bitbucket.org/mpyne/game-music-emu/"

const char gmeBackend::name[] = "Gme";

gmeConfig_t gmeConfig::m_settings;

QStringList gmeBackend::_ext;

/*****************************************************************/

size_t gmeBackend::fillBuffer(void* buffer, const size_t bufferSize)
{
    if (gme_track_ended(_emu))
        return 0;

    gme_play(_emu, (bufferSize>>1), (short*)buffer);
    return bufferSize;
}

/*****************************************************************/

void gmeConfig::loadSettings()
{
    qDebug() << "gmeConfig::loadSettings";
    m_settings.samplerate = load("Samplerate", 44100);
    m_settings.equalizer = load("Equalizer", false);
    m_settings.treble_dB = load("Treble dB", 0.0);
    m_settings.bass_freq = load("Bass freq", 15);
    m_settings.asmaPath = load("ASMA", QString());
}

void gmeConfig::saveSettings()
{
    /*if (m_settings.asmaPath.compare(load("ASMA", QString())))
    {
        qDebug() << "Reloading ASMA from " << m_settings.asmaPath;
        openAsma(m_settings.asmaPath);
    }*/

    save("Samplerate", m_settings.samplerate);
    save("Equalizer", m_settings.equalizer);
    save("Treble dB", m_settings.treble_dB);
    save("Bass freq", m_settings.bass_freq);
    save("ASMA", m_settings.asmaPath);
}

/*****************************************************************/

bool gmeBackend::init()
{
#if GME_VERSION >= 0x000602
    gme_type_t const* types = gme_type_list();

    while (gme_type_t type = *types++)
    {
        _ext << gme_type_extension(type);
    }
#else
    _ext = QString(EXT).split("|");
#endif
    return true;
}

QStringList gmeBackend::ext() { return QString(EXT).split("|"); }

gmeBackend::gmeBackend() :
    inputBackend(name),
    _emu(nullptr),
    _currentTrack(0)
#ifdef HAVE_STILVIEW
    , _stil(nullptr)
#endif
    , m_config(name)
{
    openAsma(m_config.asmaPath());
}

gmeBackend::~gmeBackend()
{
    close();
#ifdef HAVE_STILVIEW
    delete _stil;
#endif
}

bool gmeBackend::open(const QString& fileName)
{
    close();

    gme_type_t fileType;
    if (!checkRetCode(gme_identify_file(fileName.toUtf8().constData(), &fileType)))
        return false;

    qDebug() << "System " << gme_type_system(fileType);

    _emu = gme_new_emu(fileType, m_config.samplerate());
    if (_emu == nullptr)
        return false;
    if (m_config.equalizer())
    {
        gme_equalizer_t eq = { m_config.treble_dB(), m_config.bass_freq() };
        gme_set_equalizer(_emu, &eq);
    }
    if (!checkRetCode(gme_load_file(_emu, fileName.toUtf8().constData())))
        return false;
    if (!checkRetCode(gme_start_track(_emu, 0)))
        return false;

    _currentTrack = 0;

    QFileInfo fInfo(fileName);
    gme_load_m3u(_emu, QString("%1%2.m3u").arg(fInfo.canonicalPath()).arg(fInfo.completeBaseName()).toLocal8Bit().constData());

#ifdef HAVE_STILVIEW
    bool hasStilInfo = _stil && !fInfo.suffix().compare("sap", Qt::CaseInsensitive);
    if (hasStilInfo)
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
            m_metaData.addInfo(metaData::COMMENT, comment);
        }
    }
#endif

    getInfo();

    songLoaded(fileName);
    return true;
}

void gmeBackend::getInfo()
{
    gme_info_t *ti;
    const char* err = gme_track_info(_emu, &ti, _currentTrack);
    if (err)
    {
        qWarning() << "Warning: " << err;
        return;
    }

    m_metaData.addInfo(metaData::TITLE, ti->song);
    m_metaData.addInfo(metaData::ARTIST, ti->author);
    m_metaData.addInfo(gettext("copyright"), ti->copyright);
    m_metaData.addInfo(gettext("system"), ti->system);
    m_metaData.addInfo(gettext("game"), ti->game);
    m_metaData.addInfo(gettext("dumper"), ti->dumper);

    QString comment = m_metaData.getInfo(metaData::COMMENT);
    if (comment.isEmpty())
        m_metaData.addInfo(metaData::COMMENT, ti->comment);

    // length is -1 if unknown
    if (ti->length > 0)
    {
        time(ti->length);
    }
    else if (ti->loop_length > 0)
    {
        time(ti->intro_length + ti->loop_length);
    }

    gme_free_info(ti);
}

bool gmeBackend::checkRetCode(const char* error)
{
    if (error)
    {
        gme_delete(_emu);
        _emu = nullptr;
        qWarning() << "Error: " << error;
        return false;
    }
    return true;
}

void gmeBackend::close()
{
    gme_delete(_emu);
    _emu = nullptr;

    songLoaded(QString());
}

bool gmeBackend::rewind()
{
    if (_emu != nullptr)
    {
        gme_seek_samples(_emu, 0);
        return true;
    }
    return false;
}

bool gmeBackend::subtune(const unsigned int i)
{
    if ((_emu != nullptr) && (i > 0) && (i <= (unsigned int)gme_track_count(_emu)))
    {
        _currentTrack = i - 1;
        if (!checkRetCode(gme_start_track(_emu, _currentTrack)))
            return false;
        getInfo();
        return true;
    }
    return false;
}

void gmeBackend::openAsma(const QString& asmaPath)
{
#ifdef HAVE_STILVIEW
    if (asmaPath.isEmpty())
        return;

    if (_stil == nullptr)
        _stil = new STIL(ASMA_STIL, ASMA_BUGLIST);

    if (!_stil->setBaseDir(asmaPath.toLocal8Bit().constData()))
    {
        qWarning() << _stil->getErrorStr();
        utils::delPtr(_stil);
    }
#endif
}

/*****************************************************************/

#include "iconFactory.h"

#define GMESETTINGS gmeConfig::m_settings

gmeConfigFrame::gmeConfigFrame(QWidget* win) :
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
    connect(freqBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
        [this](int val) {
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
    );

    {
        QVBoxLayout *equalizerBox = new QVBoxLayout();
        QGroupBox *group = new QGroupBox(tr("Equalizer"));
        group->setCheckable(true);
        group->setToolTip(tr("Enable equalizer"));
        group->setChecked(GMESETTINGS.equalizer);
        group->setLayout(equalizerBox);
        connect(group, &QGroupBox::toggled,
            [](bool val) {
                GMESETTINGS.equalizer = val;
            }
        );

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
        connect(knob, &QDial::valueChanged,
            [tf](int val) {
                GMESETTINGS.treble_dB = val;
                tf->setNum(val);
            }
        );

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
        connect(knob, &QDial::valueChanged,
            [tf](int val) {
                GMESETTINGS.bass_freq = val;
                tf->setNum(val);
            }
        );

        matrix()->addWidget(group);
    }

    QHBoxLayout *hf = new QHBoxLayout();
    extraBottom()->addLayout(hf);

    hf->addWidget(new QLabel(tr("ASMA path:"), this));
    QLineEdit *asmaPath = new QLineEdit(this);
    asmaPath->setText(GMESETTINGS.asmaPath);
    hf->addWidget(asmaPath);
    connect(asmaPath, &QLineEdit::editingFinished,
        [asmaPath, this]() {
            QString path = asmaPath->text();
            if (!path.isEmpty() && !QFileInfo(path).exists())
            {
                QMessageBox::warning(this, tr("Warning"), tr("Path does not exists"));
            }
            else
            {
                GMESETTINGS.asmaPath = path;
            }
        }
    );
    QPushButton* button = new QPushButton(GET_ICON(icon_documentopen), tr("&Browse"), this);
    button->setToolTip(tr("Select ASMA directory"));
    hf->addWidget(button);
    connect(button, &QPushButton::clicked,
        [asmaPath, this]() {
            QString dir = QFileDialog::getExistingDirectory(this, tr("Select ASMA directory"), GMESETTINGS.asmaPath);
            if (!dir.isNull())
                GMESETTINGS.asmaPath = dir;

            asmaPath->setText(GMESETTINGS.asmaPath);
        }
    );
#ifndef HAVE_STILVIEW
     asmaPath->setEnabled(false);
     button->setEnabled(false);
#endif
}
