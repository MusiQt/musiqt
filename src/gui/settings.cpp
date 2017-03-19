/*
 *  Copyright (C) 2008-2017 Leandro Nini
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
#include "input/input.h"
#include "output/outputFactory.h"
#include "iconFactory.h"

#include <QDebug>
#include <QButtonGroup>
#include <QColorDialog>
#include <QFrame>
#include <QGroupBox>
#include <QCheckBox>
#include <QPushButton>
#include <QToolButton>
#include <QRadioButton>
#include <QStackedWidget>
#include <QSettings>

settingsWindow::settingsWindow(QWidget* win, inputConfig* i) :
    QDialog(win)
{
    setWindowTitle(tr("Settings"));

    QVBoxLayout* main = new QVBoxLayout();
    setLayout(main);

    QSizePolicy sizePol(QSizePolicy::Minimum, QSizePolicy::Expanding);
    QSizePolicy sizeMin(QSizePolicy::Expanding, QSizePolicy::Maximum);

    QHBoxLayout *horizontal = new QHBoxLayout();
    main->addLayout(horizontal);
    QVBoxLayout *buttons = new QVBoxLayout();
    horizontal->addLayout(buttons);
    QStackedWidget *switcher = new QStackedWidget(this);
    horizontal->addWidget(switcher);

    QButtonGroup *buttonGroup = new QButtonGroup(this);
    connect(buttonGroup, SIGNAL(buttonClicked(int)), switcher, SLOT(setCurrentIndex(int)));

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
    cBox->setToolTip(tr("Play all subtune"));
    cBox->setCheckState(SETTINGS->_subtunes ? Qt::Checked : Qt::Unchecked);
    connect(cBox, SIGNAL(stateChanged(int)), this, SLOT(setSubtunes(int)));
    optionLayout->addWidget(cBox);

    cBox = new QCheckBox(tr("&Auto backend"), this);
    cBox->setToolTip(tr("Automatic backend selection"));
    cBox->setCheckState(SETTINGS->_autoBk ? Qt::Checked : Qt::Unchecked);
    connect(cBox, SIGNAL(stateChanged(int)), this, SLOT(setAutobk(int)));
    optionLayout->addWidget(cBox);

#ifdef HAVE_BS2B
    cBox = new QCheckBox(tr("&Bauer stereophonic-to-binaural DSP"), this);
    cBox->setToolTip(tr("Bauer stereophonic-to-binaural DSP"));
    cBox->setCheckState(SETTINGS->_bs2b ? Qt::Checked : Qt::Unchecked);
    connect(cBox, SIGNAL(stateChanged(int)), this, SLOT(setBs2b(int)));
    optionLayout->addWidget(cBox);
#endif
#if 0
    cBox = new QCheckBox(tr("&Album art HQ resizing"), this);
    cBox->setToolTip(tr("High quality album art resizing"));
    optionLayout->addWidget(cBox);
#endif

    {
        _replayGainBox = new QVBoxLayout();
        QGroupBox *group = new QGroupBox(tr("Replaygain"));
        group->setCheckable(true);
        group->setToolTip(tr("Enable replaygain loudness normalization"));
        group->setChecked(SETTINGS->_replayGain);
        group->setLayout(_replayGainBox);
        connect(group, SIGNAL(toggled(bool)), this, SLOT(setReplaygain(bool)));

        QButtonGroup *radioGroup = new QButtonGroup(this);

        QRadioButton* radio = new QRadioButton(tr("Album gain"), this);
        radio->setToolTip(tr("Preserve album dynamics"));
        radio->setChecked(SETTINGS->_replayGainMode==0);
        _replayGainBox->layout()->addWidget(radio);
        radioGroup->addButton(radio, 0);
        radio = new QRadioButton(tr("Track gain"), this);
        radio->setToolTip(tr("All tracks equal loudness"));
        radio->setChecked(SETTINGS->_replayGainMode==1);
        _replayGainBox->layout()->addWidget(radio);
        radioGroup->addButton(radio, 1);
        _replayGainBox->addStretch(1);
        optionLayout->addWidget(group);

        connect(radioGroup, SIGNAL(buttonClicked(int)), this, SLOT(setReplaygainMode(int)));
    }
    switcher->addWidget(optionpane);

    int section = 0;

    QToolButton* button = new QToolButton(this);
    button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    button->setIcon(GET_ICON(icon_preferencesdesktop));
    button->setText(tr("General")); 
    button->setToolTip(tr("General setting"));
    button->setStatusTip("General setting"); // FIXME
    button->setCheckable(true);
    button->setChecked(true);
    button->setSizePolicy(sizePol);
    buttonGroup->addButton(button, section++);
    buttons->addWidget(button);
#if 0
    // Interface settings
    QWidget* interfacepane=new QWidget();
    QVBoxLayout* interfaceLayout = new QVBoxLayout();
    interfacepane->setLayout(interfaceLayout);
    QLabel* label = new QLabel(tr("Interface settings"), this);
    interfaceLayout->addWidget(label);
    //label->setSizePolicy(sizeMin);

    {
        QFrame* line = new QFrame();
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        interfaceLayout->addWidget(line);
    }

    QHBoxLayout *matrix=new QHBoxLayout();
    interfaceLayout->addLayout(matrix);
    label = new QLabel(tr("Alt. color"), this);
    label->setToolTip("Alternate color for lists");
    matrix->addWidget(label);
    colorLabel = new QLabel;
    colorLabel->setFrameStyle(QFrame::Sunken | QFrame::Panel);
    matrix->addWidget(colorLabel);
    QPushButton *colorButton = new QPushButton(tr("Color"));
    matrix->addWidget(colorButton);
    connect(colorButton, SIGNAL(clicked()), this, SLOT(setColor()));
    switcher->addWidget(interfacepane);

    button = new QToolButton(this);
    button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    button->setIcon(GET_ICON(icon_guioptions));
    button->setText( tr("Interface")); 
    button->setToolTip("Interface setting");
    button->setStatusTip("Interface setting"); // FIXME
    button->setCheckable(true);
    button->setSizePolicy(sizePol);
    buttonGroup->addButton(button, section++);
    buttons->addWidget(button);
#endif
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
    button->setIcon(GET_ICON(icon_card));
    button->setText(tr("Audio")); 
    button->setToolTip(tr("Audio setting"));
    button->setStatusTip("Audio setting"); // FIXME
    button->setCheckable(true);
    button->setSizePolicy(sizePol);
    buttonGroup->addButton(button, section++);
    buttons->addWidget(button);

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

    backendLayout->addWidget(i->config(backendpane));

    switcher->addWidget(backendpane);

    button = new QToolButton(this);
    button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    button->setIcon(i->icon());
    button->setText(tr("Backend")); 
    button->setToolTip(tr("Backend setting"));
    button->setStatusTip("Backend setting"); // FIXME
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

    QHBoxLayout* bottom=new QHBoxLayout();
    main->addLayout(bottom);
    QPushButton* initial = new QPushButton(GET_ICON(icon_dialogok), tr("&OK"), this);
    bottom->addWidget(initial);
    QPushButton* b = new QPushButton(GET_ICON(icon_dialogcancel), tr("&Cancel"), this);
    bottom->addWidget(b);
    initial->setFocus();
    connect(b, SIGNAL(clicked()), this, SLOT(reject()));
    connect(initial, SIGNAL(clicked()), this, SLOT(accept()));
}

void settingsWindow::setSubtunes(int val)
{
    SETTINGS->_subtunes = val == Qt::Checked;
}

void settingsWindow::setAutobk(int val)
{
    SETTINGS->_autoBk = val == Qt::Checked;
}

#ifdef HAVE_BS2B
void settingsWindow::setBs2b(int val)
{
    SETTINGS->_bs2b = val == Qt::Checked;
}
#endif

void settingsWindow::setReplaygain(bool val)
{
    SETTINGS->_replayGain = val;
}

void settingsWindow::setReplaygainMode(int val)
{
    SETTINGS->_replayGainMode = val;
}

/*****************************************************************/

settings* SETTINGS
{
    static settings s;
    return &s;
}

void settings::load(const QSettings& appSettings)
{
    _apiName=appSettings.value("Audio Settings/api", OFACTORY->name(0)).toString();
    _card=appSettings.value("Audio Settings/card").toString();
    _bits=appSettings.value("Audio Settings/bits", 16).toInt();

    _subtunes=appSettings.value("General Settings/play subtunes", false).toBool();
    _autoBk=appSettings.value("General Settings/auto backend", true).toBool();
    _replayGain=appSettings.value("General Settings/Replaygain", false).toBool();
    QString replayGainMode=appSettings.value("General Settings/Replaygain mode", "Album").toString();
    _replayGainMode=(!replayGainMode.compare("Track")) ? 1 : 0;

#ifdef HAVE_BS2B
    _bs2b=appSettings.value("General Settings/Bauer DSP", false).toBool();
#endif
}

void settings::save(QSettings& appSettings)
{
    appSettings.setValue("Audio Settings/api", _apiName);
    appSettings.setValue("Audio Settings/card", _card);
    appSettings.setValue("Audio Settings/bits", _bits);

    appSettings.setValue("General Settings/play subtunes", _subtunes);
    appSettings.setValue("General Settings/auto backend", _autoBk);
    appSettings.setValue("General Settings/Replaygain", _replayGain);
    appSettings.setValue("General Settings/Replaygain mode", _replayGainMode==0?"Album":"Track");
#ifdef HAVE_BS2B
    appSettings.setValue("General Settings/Bauer DSP", _bs2b);
#endif
}
