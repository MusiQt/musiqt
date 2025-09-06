/*
 *  Copyright (C) 2008-2023 Leandro Nini
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

#include "settings.h"

#include "audio.h"
#include "inputConfig.h"
#include "inputFactory.h"
#include "iconFactory.h"

#ifdef HAVE_LASTFM
#  include "lastfm.h"
#endif

#include <QDebug>
#include <QButtonGroup>
#include <QEvent>
#include <QFrame>
#include <QGroupBox>
#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>
#include <QToolButton>
#include <QRadioButton>
#include <QStackedWidget>
#include <QSettings>
#include <QStatusTipEvent>
#include <QMainWindow>
#include <QStatusBar>
#include <QHBoxLayout>

settingsWindow::settingsWindow(QWidget* win, const QString& bkName) :
    QDialog(win)
{
    setWindowTitle(tr("Settings"));

    QVBoxLayout* main = new QVBoxLayout();
    setLayout(main);

    QSizePolicy sizePol(QSizePolicy::Minimum, QSizePolicy::Fixed);
    //QSizePolicy sizeMin(QSizePolicy::Expanding, QSizePolicy::Maximum);

    QHBoxLayout *horizontal = new QHBoxLayout();
    main->addLayout(horizontal);
    QVBoxLayout *buttons = new QVBoxLayout();
    horizontal->addLayout(buttons);
    QStackedWidget *switcher = new QStackedWidget(this);
    horizontal->addWidget(switcher);

    QButtonGroup *buttonGroup = new QButtonGroup(this);
    connect(buttonGroup, &QButtonGroup::idClicked, switcher, &QStackedWidget::setCurrentIndex);

    // General settings
    QWidget* optionpane = new QWidget();
    QVBoxLayout* optionLayout = new QVBoxLayout();
    optionpane->setLayout(optionLayout);
    optionLayout->addWidget(new QLabel(tr("General settings"), this));

    {
        QFrame* line = new QFrame();
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        optionLayout->addWidget(line);
    }

    QCheckBox* cBox = new QCheckBox(tr("&Play subtunes"), this);
    cBox->setToolTip(tr("Play all subtunes"));
    cBox->setCheckState(SETTINGS->m_subtunes ? Qt::Checked : Qt::Unchecked);
    connect(cBox,
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
            &QCheckBox::checkStateChanged,
        [](Qt::CheckState val)
#else
            &QCheckBox::stateChanged,
        [](int val)
#endif
        {
            SETTINGS->m_subtunes = (val == Qt::Checked);
        }
    );
    optionLayout->addWidget(cBox);

    cBox = new QCheckBox(tr("&Bauer stereophonic-to-binaural DSP"), this);
    cBox->setToolTip(tr("Bauer stereophonic-to-binaural DSP"));
    cBox->setCheckState(SETTINGS->m_bs2b ? Qt::Checked : Qt::Unchecked);
    connect(cBox,
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
            &QCheckBox::checkStateChanged,
        [](Qt::CheckState val)
#else
            &QCheckBox::stateChanged,
        [](int val)
#endif
        {
            SETTINGS->m_bs2b = (val == Qt::Checked);
        }
    );
    optionLayout->addWidget(cBox);
#ifndef HAVE_BS2B
    cBox->setDisabled(true);
#endif

    cBox = new QCheckBox(tr("&Use system icons"), this);
    cBox->setToolTip(tr("Use icons from system theme (on next restart)"));
    cBox->setCheckState(SETTINGS->m_themeIcons ? Qt::Checked : Qt::Unchecked);
    connect(cBox,
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
            &QCheckBox::checkStateChanged,
        [](Qt::CheckState val)
#else
        &QCheckBox::stateChanged,
        [](int val)
#endif
        {
            SETTINGS->m_themeIcons = (val == Qt::Checked);
        }
    );
    optionLayout->addWidget(cBox);
#ifndef Q_OS_LINUX
    cBox->setDisabled(true);
#endif

    {
        QVBoxLayout *replayGainBox = new QVBoxLayout();
        QGroupBox *group = new QGroupBox(tr("Replaygain"));
        group->setCheckable(true);
        group->setToolTip(tr("Enable replaygain loudness normalization"));
        group->setChecked(SETTINGS->m_replayGain);
        group->setLayout(replayGainBox);
        connect(group, &QGroupBox::toggled,
            [](bool val)
            {
                SETTINGS->m_replayGain = val;
            }
        );

        QButtonGroup *radioGroup = new QButtonGroup(this);

        QRadioButton* radio = new QRadioButton(tr("Album gain"), this);
        radio->setToolTip(tr("Preserve album dynamics"));
        radio->setChecked(SETTINGS->m_replayGainMode==settings::rg_t::Album);
        replayGainBox->layout()->addWidget(radio);
        radioGroup->addButton(radio, 0);
        radio = new QRadioButton(tr("Track gain"), this);
        radio->setToolTip(tr("All tracks equal loudness"));
        radio->setChecked(SETTINGS->m_replayGainMode==settings::rg_t::Track);
        replayGainBox->layout()->addWidget(radio);
        radioGroup->addButton(radio, 1);
        replayGainBox->addStretch(1);
        optionLayout->addWidget(group);

        connect(radioGroup, &QButtonGroup::idClicked,
            [](int val)
            {
                SETTINGS->m_replayGainMode = val == 0 ? settings::rg_t::Album : settings::rg_t::Track;
            }
        );
    }
    switcher->addWidget(optionpane);

    optionLayout->addStretch();

    int section = 0;

    QToolButton* button = new QToolButton(this);
    button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    button->setIcon(GET_ICON(icon_preferencesdesktop));
    button->setText(tr("General")); 
    button->setToolTip(tr("General setting"));
    button->setStatusTip("General setting");
    button->setCheckable(true);
    button->setChecked(true);
    button->setSizePolicy(sizePol);
    buttonGroup->addButton(button, section++);
    buttons->addWidget(button);

    // Audio settings
    QWidget* audiopane = new QWidget();
    QVBoxLayout* audioLayout = new QVBoxLayout();
    audiopane->setLayout(audioLayout);
    audioLayout->addWidget(new QLabel(tr("Audio settings"), this));

    {
        QFrame* line = new QFrame();
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        audioLayout->addWidget(line);
    }

    audioLayout->addWidget(new audioConfig(audiopane));
    switcher->addWidget(audiopane);

    button = new QToolButton(this);
    button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    button->setIcon(GET_ICON(icon_audiocard));
    button->setText(tr("Audio")); 
    button->setToolTip(tr("Audio setting"));
    button->setStatusTip("Audio setting");
    button->setCheckable(true);
    button->setSizePolicy(sizePol);
    buttonGroup->addButton(button, section++);
    buttons->addWidget(button);

    audioLayout->addStretch();
#ifdef HAVE_LASTFM
    // Last.fm settings
    QWidget* lastfmpane = new QWidget();
    QVBoxLayout* lastfmLayout = new QVBoxLayout();
    lastfmpane->setLayout(lastfmLayout);
    lastfmLayout->addWidget(new QLabel(tr("Last.fm settings"), this));

    {
        QFrame* line = new QFrame();
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        lastfmLayout->addWidget(line);
    }

    lastfmLayout->addWidget(new lastfmConfig(lastfmpane));
    switcher->addWidget(lastfmpane);

    button = new QToolButton(this);
    button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    button->setIcon(GET_ICON(icon_lastfm));
    button->setText(tr("Last.fm")); 
    button->setToolTip(tr("Last.fm setting"));
    button->setStatusTip("Last.fm setting");
    button->setCheckable(true);
    button->setSizePolicy(sizePol);
    buttonGroup->addButton(button, section++);
    buttons->addWidget(button);

    lastfmLayout->addStretch();
#endif
    // Backend settings
    QWidget* backendpane = new QWidget();
    QVBoxLayout* backendLayout = new QVBoxLayout();
    backendpane->setLayout(backendLayout);
    backendLayout->addWidget(new QLabel(tr("Backend settings"), this));

    {
        QFrame* line = new QFrame();
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        backendLayout->addWidget(line);
    }

    QStackedWidget *beSwitcher = new QStackedWidget();

    {
        QComboBox *backends = new QComboBox();
        for (int i=0; i<IFACTORY->num(); i++)
        {
            backends->addItem(IFACTORY->name(i));
            inputConfig *ic = IFACTORY->getConfig(i);
            backends->setItemIcon(i, ic->icon());
            m_inputConfigs.append(ic);
            beSwitcher->addWidget(ic->config());
        }
        connect(backends, QOverload<int>::of(&QComboBox::currentIndexChanged), beSwitcher, &QStackedWidget::setCurrentIndex);

        backends->setCurrentText(bkName);

        QWidget *w = new QWidget(this);
        QHBoxLayout *backendSelection = new QHBoxLayout(w);
        backendSelection->addWidget(new QLabel(tr("Backend")));
        backendSelection->addWidget(backends);
        backendLayout->addWidget(w);
    }

    backendLayout->addWidget(beSwitcher);

    backendLayout->addStretch();

    switcher->addWidget(backendpane);

    button = new QToolButton(this);
    button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    button->setIcon(GET_ICON(icon_backend));
    button->setText(tr("Backend")); 
    button->setToolTip(tr("Backend settings"));
    button->setStatusTip("Backend settings");
    button->setCheckable(true);
    button->setSizePolicy(sizePol);
    buttonGroup->addButton(button, section++);
    buttons->addWidget(button);

    {
        QFrame* line = new QFrame();
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        main->addWidget(line);
    }

    QHBoxLayout* bottom = new QHBoxLayout();
    main->addLayout(bottom);
    QPushButton* initial = new QPushButton(GET_ICON(icon_dialogok), tr("&OK"), this);
    bottom->addWidget(initial);
    QPushButton* b = new QPushButton(GET_ICON(icon_dialogcancel), tr("&Cancel"), this);
    bottom->addWidget(b);
    initial->setFocus();
    connect(b, &QPushButton::clicked,
        [this]()
        {
            //SETTINGS->load();
            for (inputConfig* ic: m_inputConfigs)
            {
                ic->loadSettings();
                delete ic;
            }
            reject();
        }
    );
    connect(initial, &QPushButton::clicked,
        [this]()
        {
            //SETTINGS->save();
            for (inputConfig* ic: m_inputConfigs)
            {
                ic->saveSettings();
                delete ic;
            }
            accept();
        }
    );

    buttons->addStretch();

    layout()->setSizeConstraint(QLayout::SetFixedSize);
}

bool settingsWindow::event(QEvent *e)
{
    if (e->type() == QEvent::StatusTip)
    {
        QStatusTipEvent *ev = (QStatusTipEvent*)e;
        ((QMainWindow*)parentWidget())->statusBar()->showMessage(ev->tip());
        return true;
    }
    return QDialog::event(e);
}

/*****************************************************************/

settings* SETTINGS
{
    static settings s;
    return &s;
}

void settings::load(const QSettings& appSettings)
{
    m_card = appSettings.value(config::AUDIO_CARD).toString();
    m_bits = appSettings.value(config::AUDIO_BITS, 16).toInt();

    m_bufLen = appSettings.value(config::AUDIO_BUFFERLEN, 500).toUInt();

    m_subtunes = appSettings.value(config::GENERAL_SUBTUNES, false).toBool();
    m_replayGain = appSettings.value(config::GENERAL_REPLAYGAIN, false).toBool();
    QString replayGainMode=appSettings.value(config::GENERAL_RG_MODE, "Album").toString();
    m_replayGainMode = (!replayGainMode.compare("Album")) ? settings::rg_t::Album : settings::rg_t::Track;

    m_bs2b=appSettings.value(config::GENERAL_BAUERDSP, false).toBool();
    m_themeIcons=appSettings.value(config::GENERAL_ICONTHEME, false).toBool();
}

void settings::save(QSettings& appSettings)
{
    appSettings.setValue(config::AUDIO_CARD, m_card);
    appSettings.setValue(config::AUDIO_BITS, m_bits);

    appSettings.setValue(config::AUDIO_BUFFERLEN, m_bufLen);

    appSettings.setValue(config::GENERAL_SUBTUNES, m_subtunes);
    appSettings.setValue(config::GENERAL_REPLAYGAIN, m_replayGain);
    appSettings.setValue(config::GENERAL_RG_MODE, (m_replayGainMode == settings::rg_t::Album) ? "Album" : "Track");
    appSettings.setValue(config::GENERAL_BAUERDSP, m_bs2b);
    appSettings.setValue(config::GENERAL_ICONTHEME, m_themeIcons);
}
