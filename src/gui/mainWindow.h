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
class player;

class QMenu;
class QAction;
class QToolBar;

class mainWindow : public QMainWindow
{
    Q_OBJECT

public:
    mainWindow(player* p, QWidget *parent = 0);
    ~mainWindow();

    void init(const char* arg);

signals:
    void setScrobbling(bool scrobble);

public:
    void onMessage(QString msg);

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    void onCompact();
    void iconActivated(QSystemTrayIcon::ActivationReason reason);
    void onAbout();
    void onConfig();
    void onInfo();
    void setPlayButton();

    void setDisplay();
    void clearDisplay(const QString& text);

    void createTrayIcon();
    void createActions();
    void setPlayMode(QAction *action, bool mode);

    QToolBar *createControlBar();
    QToolBar *createSecondaryBar();
    QToolBar *createInfoBar();

private:
    player *m_player;

    centralFrame *m_cFrame;

    QSystemTrayIcon *m_trayIcon;
    QAction *m_trayStatus;
    QLabel *m_statusLed;
    QLabel *m_subtunes;
    infoLabel *m_songInfo;
    timeLabel *m_songTime;
    timeDisplay *m_timeDisplay;
    infoDialog *m_infoDialog;

    QAction *m_aboutAction;
    QAction *m_prevAction;
    QAction *m_nextAction;
    QAction *m_playAction;
    QAction *m_stopAction;
    QAction *m_infoAction;
    QAction *m_quitAction;

    QSize m_windowSize;

    QSettings m_settings;
};

#endif
