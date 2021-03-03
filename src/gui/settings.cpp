/*
 *  Copyright (C) 2008-2021 Leandro Nini
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
#include "input/inputFactory.h"
#include "iconFactory.h"

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

settingsWindow::settingsWindow(QWidget* win) :
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
    cBox->setToolTip(tr("Play all subtunes"));
    cBox->setCheckState(SETTINGS->_subtunes ? Qt::Checked : Qt::Unchecked);
    connect(cBox, SIGNAL(stateChanged(int)), this, SLOT(setSubtunes(int)));
    optionLayout->addWidget(cBox);

    cBox = new QCheckBox(tr("&Bauer stereophonic-to-binaural DSP"), this);
    cBox->setToolTip(tr("Bauer stereophonic-to-binaural DSP"));
    cBox->setCheckState(SETTINGS->_bs2b ? Qt::Checked : Qt::Unchecked);
    connect(cBox, SIGNAL(stateChanged(int)), this, SLOT(setBs2b(int)));
    optionLayout->addWidget(cBox);
#ifndef HAVE_BS2B
    cBox->setDisabled(true);
#endif

    {
        QVBoxLayout *replayGainBox = new QVBoxLayout();
        QGroupBox *group = new QGroupBox(tr("Replaygain"));
        group->setCheckable(true);
        group->setToolTip(tr("Enable replaygain loudness normalization"));
        group->setChecked(SETTINGS->_replayGain);
        group->setLayout(replayGainBox);
        connect(group, SIGNAL(toggled(bool)), this, SLOT(setReplaygain(bool)));

        QButtonGroup *radioGroup = new QButtonGroup(this);

        QRadioButton* radio = new QRadioButton(tr("Album gain"), this);
        radio->setToolTip(tr("Preserve album dynamics"));
        radio->setChecked(SETTINGS->_replayGainMode==0);
        replayGainBox->layout()->addWidget(radio);
        radioGroup->addButton(radio, 0);
        radio = new QRadioButton(tr("Track gain"), this);
        radio->setToolTip(tr("All tracks equal loudness"));
        radio->setChecked(SETTINGS->_replayGainMode==1);
        replayGainBox->layout()->addWidget(radio);
        radioGroup->addButton(radio, 1);
        replayGainBox->addStretch(1);
        optionLayout->addWidget(group);

        connect(radioGroup, SIGNAL(buttonClicked(int)), this, SLOT(setReplaygainMode(int)));
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
    button->setIcon(GET_ICON(icon_card));
    button->setText(tr("Audio")); 
    button->setToolTip(tr("Audio setting"));
    button->setStatusTip("Audio setting");
    button->setCheckable(true);
    button->setSizePolicy(sizePol);
    buttonGroup->addButton(button, section++);
    buttons->addWidget(button);

    audioLayout->addStretch();

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
            inputConfigs.append(ic);
            beSwitcher->addWidget(ic->config());
        }
        connect(backends, SIGNAL(currentIndexChanged(int)), beSwitcher, SLOT(setCurrentIndex(int)));

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
    connect(b, SIGNAL(clicked()), this, SLOT(onReject()));
    connect(initial, SIGNAL(clicked()), this, SLOT(onAccept()));

    buttons->addStretch();

    layout()->setSizeConstraint(QLayout::SetFixedSize);
}

void settingsWindow::onAccept()
{
    //SETTINGS->save();
    for (inputConfig* ic: inputConfigs)
    {
        ic->saveSettings();
        delete ic;
    }
    accept();
}

void settingsWindow::onReject()
{
    //SETTINGS->load();
    for (inputConfig* ic: inputConfigs)
    {
        ic->loadSettings();
        delete ic;
    }
    reject();
}

void settingsWindow::setSubtunes(int val)
{
    SETTINGS->_subtunes = val == Qt::Checked;
}

void settingsWindow::setBs2b(int val)
{
    SETTINGS->_bs2b = val == Qt::Checked;
}

void settingsWindow::setReplaygain(bool val)
{
    SETTINGS->_replayGain = val;
}

void settingsWindow::setReplaygainMode(int val)
{
    SETTINGS->_replayGainMode = val;
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
    _card = appSettings.value("Audio Settings/card").toString();
    _bits = appSettings.value("Audio Settings/bits", 16).toInt();

    _subtunes = appSettings.value("General Settings/play subtunes", false).toBool();
    _replayGain = appSettings.value("General Settings/Replaygain", false).toBool();
    QString replayGainMode=appSettings.value("General Settings/Replaygain mode", "Album").toString();
    _replayGainMode = (!replayGainMode.compare("Track")) ? 1 : 0;

    _bs2b=appSettings.value("General Settings/Bauer DSP", false).toBool();
}

void settings::save(QSettings& appSettings)
{
    appSettings.setValue("Audio Settings/card", _card);
    appSettings.setValue("Audio Settings/bits", _bits);

    appSettings.setValue("General Settings/play subtunes", _subtunes);
    appSettings.setValue("General Settings/Replaygain", _replayGain);
    appSettings.setValue("General Settings/Replaygain mode", (_replayGainMode == 0) ? "Album" : "Track");
    appSettings.setValue("General Settings/Bauer DSP", _bs2b);
}
