/*
 *  Copyright (C) 2013-2021 Leandro Nini
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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "audio.h"

#include <QMainWindow>
#include <QApplication>
#include <QPushButton>
#include <QLabel>
#include <QSystemTrayIcon>
#include <QSettings>

class centralFrame;
class bookmark;
class playlist;
class timeLabel;
class timeDisplay;
class infoLabel;
class infoDialog;
class metaData;

class QMenu;
class QAction;
class QToolBar;

class mainWindow : public QMainWindow
{
public:
    explicit mainWindow(QWidget *parent = 0);
    ~mainWindow();

    void init(const char* arg);

public:
    void onMessage(QString msg);

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    void onCompact();
    void onPlaymode();
    void iconActivated(QSystemTrayIcon::ActivationReason reason);
    void onAbout();
    void onConfig();
    void onInfo();
    void setPlayButton(state_t state);

    void setDisplay(input* i);
    void clearDisplay(bool loading);

    void updateTime(int seconds);
    void setInfo(const metaData* mtd);

    void onCmdVol(int vol);
    void onPrevSubtune();
    void onNextSubtune();
    void onStatusbarChanged(const QString &message);

    void createTrayIcon();
    void createActions();
    void setPlayMode(QAction *action, bool mode);

    QToolBar *createControlBar();
    QToolBar *createSecondaryBar();
    QToolBar *createInfoBar();

private:
    centralFrame *cFrame;

    QSystemTrayIcon *trayIcon;
    QAction *_mStatus;
    QLabel *led;
    infoLabel *_songInfo;
    timeLabel *_songTime;
    timeDisplay *_timeDisplay;
    infoDialog *_infoDialog;
    QLabel *subtunes;

    QAction *aboutAction;
    QAction *prevAction;
    QAction *nextAction;
    QAction *playAction;
    QAction *stopAction;
    QAction *infoAction;
    QAction *quitAction;

    QSize windowSize;

    QSettings settings;
};

#endif
