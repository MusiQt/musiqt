/*
 *  Copyright (C) 2013-2017 Leandro Nini
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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "mainWindow.h"

#include "icons.h"
#include "iconFactory.h"
#include "aboutDialog.h"
#include "infoDialog.h"
#include "infoLabel.h"
#include "settings.h"
#include "timeDisplay.h"
#include "centralFrame.h"

#include <QStatusBar>
#include <QAction>
#include <QDir>
#include <QToolButton>
#include <QAbstractItemDelegate>
#include <QFileInfo>
#include <QLayout>
#include <QDial>
#include <QMenu>
#include <QToolBar>
#include <QLCDNumber>
#include <QBitmap>

#ifdef _WIN32
#  include <windows.h>
#endif

#include "ledRed.xpm"
#include "ledYellow.xpm"
#include "ledGreen.xpm"

mainWindow::mainWindow(QWidget *parent) :
    QMainWindow(parent),
    _infoDialog(nullptr)
{
    createActions();

    //setAttribute(Qt::WA_AlwaysShowToolTips);
    statusBar()->showMessage(PACKAGE_STRING);
    connect(statusBar(), SIGNAL(messageChanged(const QString&)), this, SLOT(onStatusbarChanged(const QString&)));

    QToolButton *about = new QToolButton();
    about->setAutoRaise(true);
    about->setDefaultAction(aboutAction);
    statusBar()->addPermanentWidget(about);

    QToolButton *quit = new QToolButton();
    quit->setAutoRaise(true);
    quit->setDefaultAction(quitAction);
    statusBar()->addPermanentWidget(quit);

    cFrame = new centralFrame(this);
    setCentralWidget(cFrame);

    connect(playAction, SIGNAL(triggered()), cFrame, SLOT(onCmdPlayPauseSong()));
    connect(stopAction, SIGNAL(triggered()), cFrame, SLOT(onCmdStopSong()));
    connect(prevAction, SIGNAL(triggered()), cFrame, SLOT(onCmdPrevSong()));
    connect(nextAction, SIGNAL(triggered()), cFrame, SLOT(onCmdNextSong()));

    connect(cFrame, SIGNAL(updateTime(int)), this, SLOT(updateTime(int)));
    connect(cFrame, SIGNAL(stateChanged(state_t)), this, SLOT(setPlayButton(state_t)));
    connect(cFrame, SIGNAL(setDisplay(input*)), this, SLOT(setDisplay(input*)));
    connect(cFrame, SIGNAL(clearDisplay(bool)), this, SLOT(clearDisplay(bool)));
    connect(cFrame, SIGNAL(setInfo(const metaData*)), this, SLOT(setInfo(const metaData*)));

    addToolBar(createControlBar());
    addToolBarBreak();
    addToolBar(createSecondaryBar());  
    addToolBar(Qt::BottomToolBarArea, createInfoBar());

    setWindowTitle(PACKAGE_NAME);
    setWindowIcon(GET_ICON(icon_logo32));

    if (QSystemTrayIcon::isSystemTrayAvailable())
    {
        createTrayIcon();

        connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
                this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));

        trayIcon->setIcon(GET_ICON(icon_logo32));
        trayIcon->setToolTip(PACKAGE_STRING);
        trayIcon->show();
    }

    qApp->setStyleSheet("QMainWindow > QPushButton,QToolButton { margin:0; padding:0; }");

    // Read settings
    SETTINGS->load(settings);

    QPoint pos = settings.value("General Settings/pos").toPoint();
    QSize size = settings.value("General Settings/size").toSize();

    resize(size);
    move(pos);
}

mainWindow::~mainWindow()
{
    // Save Settings
    SETTINGS->save(settings);

    if (!centralWidget()->isHidden())
    {
        settings.setValue("General Settings/size", size());
    }
    settings.setValue("General Settings/pos", pos());
    settings.setValue("General Settings/playlist mode", cFrame->getPlayMode());
}

void mainWindow::init(const char* arg)
{
    // Look at command line arguments
    if (arg != nullptr)
    {
#if defined (_WIN32) && defined (UNICODE)
        int narg;
        LPWSTR* argw = CommandLineToArgvW(GetCommandLineW(), &narg);
        const QString message = QString::fromWCharArray(argw[1]);
#else
        const QString message(arg);
#endif
        cFrame->setFile(QDir(message).absolutePath(), true);
    }
    else
    {
        cFrame->setFile(settings.value("General Settings/file").toString(), false);
    }
}

QToolBar *mainWindow::createControlBar()
{
    _timeDisplay = new timeDisplay(this);

    QToolBar *controlBar = new QToolBar("controlBar", this);
    controlBar->addAction(prevAction);
    controlBar->addAction(stopAction);
    controlBar->addAction(playAction);
    controlBar->addAction(nextAction);

    controlBar->addWidget(_timeDisplay);

    QAction *act = controlBar->addAction(GET_ICON(icon_viewcompact), tr("Compact"));
    act->setToolTip(tr("Compact"));
    act->setStatusTip(tr("Switch to compact view"));
    connect(act, SIGNAL(triggered()), this, SLOT(onCompact()));

    act = controlBar->addAction(GET_ICON(icon_preferencesdesktop), "Settings");
    act->setToolTip(tr("Settings"));
    act->setStatusTip(tr("Program settings"));
    connect(act, SIGNAL(triggered()), this, SLOT(onConfig()));

    controlBar->setMovable(false);

    return controlBar;
}

QToolBar *mainWindow::createSecondaryBar()
{
    QAction *prevtune = new QAction(GET_ICON(icon_goprevious), tr("Previous"), this);
    prevtune->setToolTip(tr("Previous"));
    prevtune->setStatusTip(tr("Go to previous subtune"));
    connect(prevtune, SIGNAL(triggered()), this, SLOT(onPrevSubtune()));

    QAction *nexttune = new QAction(GET_ICON(icon_gonext), tr("Next"), this);
    nexttune->setToolTip(tr("Next"));
    nexttune->setStatusTip(tr("Go to next subtune"));
    connect(nexttune, SIGNAL(triggered()), this, SLOT(onNextSubtune()));

    QAction *playlist = new QAction(GET_ICON(icon_playlist), tr("Playlist"), this);
    const bool playMode = settings.value("General Settings/playlist mode", true).toBool();
    cFrame->setPlayMode(playMode);
    setPlayMode(playlist, playMode);
    connect(playlist, SIGNAL(triggered()), this, SLOT(onPlaymode()));

    subtunes = new QLabel(this);
    subtunes->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    subtunes->setText("00/00");
    subtunes->setStatusTip(tr("Subtunes"));

    QLabel *songs = new QLabel(this);
    songs->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    songs->setText("00/00");
    songs->setStatusTip(tr("Songs"));
    connect(cFrame, SIGNAL(songUpdated(const QString&)), songs, SLOT(setText(const QString&)));

    led = new QLabel(this);
    led->setPixmap(QPixmap(ledRed));
    led->setStatusTip(tr("Stopped"));

    QWidget* empty = new QWidget(this);
    empty->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    QDial *volume = new QDial(this);
    //volume->setNotchesVisible(true);
    volume->setRange(0, 100);
    volume->setStatusTip(tr("Volume"));
    volume->setValue(cFrame->volume());
    connect(volume, SIGNAL(valueChanged(int)), this, SLOT(onCmdVol(int)));

    QToolBar *secondaryBar = new QToolBar("secondaryBar", this);
    secondaryBar->addAction(prevtune);
    secondaryBar->addWidget(subtunes);
    secondaryBar->addAction(nexttune);
    secondaryBar->addWidget(led);
    secondaryBar->addWidget(songs);
    secondaryBar->addAction(playlist);
    secondaryBar->addWidget(empty);
    secondaryBar->addWidget(volume);

    const int height = secondaryBar->height();
    volume->setMaximumSize(height, height);

    secondaryBar->setMovable(false);

    return secondaryBar;
}

QToolBar *mainWindow::createInfoBar()
{
    _songInfo = new infoLabel(this);
    _songInfo->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    _songInfo->setText("--");
    _songInfo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    _songTime = new timeLabel(this);

    QToolBar *infoBar = new QToolBar("infoBar", this);
    infoBar->addAction(infoAction);
    infoBar->addWidget(_songInfo);
    infoBar->addWidget(_songTime);

    infoBar->setMovable(false);

    return infoBar;
}

void mainWindow::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason)
    {
    case QSystemTrayIcon::Trigger:
        if (isHidden())
        {
            show();
            raise();
            activateWindow();
        }
        else
        {
            hide();
        }
        break;
    default:
        break;
    }
}

void mainWindow::createActions()
{
    aboutAction = new QAction(GET_ICON(icon_logo16), tr("&About"), this);
    aboutAction->setToolTip(tr("About"));
    aboutAction->setStatusTip(tr("About"));
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(onAbout()));

    prevAction = new QAction(GET_ICON(icon_mediaskipbackward), tr("P&revious"), this);
    prevAction->setToolTip(tr("Previous"));
    prevAction->setStatusTip(tr("Go to previous song"));

    nextAction = new QAction(GET_ICON(icon_mediaskipforward), tr("&Next"), this);
    nextAction->setToolTip(tr("Next"));
    nextAction->setStatusTip(tr("Go to next song"));

    playAction = new QAction(GET_ICON(icon_mediaplaybackstart), tr("&Play"), this);
    playAction->setToolTip(tr("Play"));
    playAction->setStatusTip(tr("Play song"));

    stopAction = new QAction(GET_ICON(icon_mediaplaybackstop), tr("&Stop"), this);
    stopAction->setToolTip(tr("Stop"));
    stopAction->setStatusTip(tr("Stop"));

    infoAction = new QAction(GET_ICON(icon_helpabout), tr("&Info"), this);
    infoAction->setToolTip(tr("Info"));
    infoAction->setStatusTip(tr("Song info"));
    connect(infoAction, SIGNAL(triggered()), this, SLOT(onInfo()));

    quitAction = new QAction(GET_ICON(icon_applicationexit), tr("&Quit"), this);
    quitAction->setToolTip(tr("Exit"));
    quitAction->setStatusTip(tr("Exit"));
    connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
}

void mainWindow::createTrayIcon()
{
    trayIconMenu = new QMenu(this);
    _mStatus = new QAction(trayIconMenu);
    _mStatus->setText(tr("Stopped"));
    _mStatus->setIcon(QPixmap(ledRed));
    //_mStatus->setEnabled(false);

    trayIconMenu->addAction(_mStatus);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(playAction);
    trayIconMenu->addAction(stopAction);
    trayIconMenu->addAction(prevAction);
    trayIconMenu->addAction(nextAction);
    trayIconMenu->addAction(infoAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayIconMenu);
}

void mainWindow::onCompact()
{
    QAction* act = (QAction*)sender();
    if (centralWidget()->isHidden())
    {
        act->setIcon(GET_ICON(icon_viewcompact));
        act->setToolTip(tr("Compact"));
        act->setStatusTip(tr("Switch to compact view"));

        centralWidget()->show();
        statusBar()->setSizeGripEnabled(true);
        setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
        resize(windowSize);
    }
    else
    {
        act->setIcon(GET_ICON(icon_viewfull));
        act->setToolTip(tr("Full mode"));
        act->setStatusTip(tr("Switch to full UI mode"));

        if (isMaximized())
            return;
        windowSize = size();
        centralWidget()->hide();
        statusBar()->setSizeGripEnabled(false);
        adjustSize();
        setFixedSize(size());
    }
}

void mainWindow::onAbout()
{
    aboutDialog *about = new aboutDialog(this);
    about->setAttribute(Qt::WA_DeleteOnClose);
    about->exec();
}

void mainWindow::onConfig()
{
    settingsWindow *config = new settingsWindow(this, cFrame->getInput());
    config->setAttribute(Qt::WA_DeleteOnClose);
    int result = config->exec();
    switch (result)
    {
    case QDialog::Accepted:
        // emit stop();
        cFrame->onCmdStopSong();
        SETTINGS->save(settings);
        cFrame->setOpts();
        break;
    case QDialog::Rejected:
        SETTINGS->load(settings);
        cFrame->getInput()->loadSettings();
        break;
    }
}

void mainWindow::onInfo()
{
    _infoDialog = new infoDialog(this);
    _infoDialog->setInfo(cFrame->getInput()->getMetaData());
    _infoDialog->setAttribute(Qt::WA_QuitOnClose, false);
    _infoDialog->show();
    connect(_infoDialog, SIGNAL(accepted()), this, SLOT(onCloseInfo()));
}

void mainWindow::onCloseInfo()
{
    qDebug("onCloseInfo");
    // FIXME not called
    delPtr(_infoDialog);
}

void mainWindow::setPlayButton(state_t state)
{
    QString label;

    {
        const char* icon;

        if (state != state_t::PLAY)
        {
            icon = icon_mediaplaybackstart;
            label = tr("Play");
        }
        else
        {
            icon = icon_mediaplaybackpause;
            label = tr("Pause");
        }

        playAction->setIcon(GET_ICON(icon));
        playAction->setToolTip(label);
        playAction->setStatusTip(label);
    }

    {
        const char** icon;
        switch (state)
        {
        case state_t::STOP:
            icon = ledRed;
            label = tr("Stopped");
            break;
        case state_t::PLAY:
            icon = ledGreen;
            label = tr("Playing");
            break;
        case state_t::PAUSE:
            icon = ledYellow;
            label = tr("Paused");
            break;
        }

        _mStatus->setIcon(QPixmap(icon));
        _mStatus->setText(label);
        led->setPixmap(QPixmap(icon));
        led->setStatusTip(label);
    }
}

void mainWindow::setDisplay(input* i)
{
    _timeDisplay->setTime(0);
    _songTime->setTime(i->time());

    subtunes->setText(QString("%1/%2").arg(i->subtune()).arg(i->subtunes()));

    const metaData* data = i->getMetaData();
    QString songTitle = data->getInfo(metaData::TITLE);
    if (songTitle.isEmpty())
        songTitle = QFileInfo(i->songLoaded()).fileName();

    setWindowTitle(songTitle);

    QString artist = data->getInfo(metaData::ARTIST);
    if (!artist.isEmpty())
        artist.append('\n');

    trayIcon->setToolTip(artist+songTitle);

    setInfo(data);

    if (_infoDialog != nullptr)
        _infoDialog->setInfo(i->getMetaData());
}

void mainWindow::clearDisplay(bool loading)
{
    _songInfo->setText(loading ? tr("Loading...") : QString::null);
    _songInfo->setToolTip(QString::null);
    setWindowTitle(QString(PACKAGE_STRING));
    trayIcon->setToolTip(PACKAGE_STRING);

    _songTime->reset();
    _timeDisplay->reset();
}

void mainWindow::setInfo(const metaData* mtd)
{
    _songInfo->setInfo(mtd);
}

void mainWindow::updateTime(int seconds)
{
    _timeDisplay->setTime(seconds);
}

void mainWindow::onPlaymode()
{
    const bool playMode = !cFrame->getPlayMode();
    setPlayMode((QAction*)sender(), playMode);
    cFrame->setPlayMode(playMode);
}

void mainWindow::setPlayMode(QAction *action, bool mode)
{
    if (mode)
    {
        action->setToolTip(tr("Song mode"));
        action->setStatusTip(tr("Switch to song mode"));
        action->setIcon(GET_ICON(icon_playlist));
    }
    else
    {
        action->setToolTip(tr("Playlist mode"));
        action->setStatusTip(tr("Switch to playlist mode"));
        action->setIcon(GET_ICON(icon_song));
    }
}

void mainWindow::onCmdVol(int vol)
{
    cFrame->setVolume(vol);
}

void mainWindow::onPrevSubtune()
{
    cFrame->changeSubtune(centralFrame::ID_PREV);
}

void mainWindow::onNextSubtune()
{
    cFrame->changeSubtune(centralFrame::ID_NEXT);
}

void mainWindow::onStatusbarChanged(const QString &message)
{
    if (message.isNull())
        statusBar()->showMessage(PACKAGE_STRING);
}

void mainWindow::keyPressEvent(QKeyEvent *event)
{
    switch (event->key())
    {
    case Qt::Key_MediaPlay: cFrame->onCmdPlayPauseSong(); event->accept(); break;
    case Qt::Key_MediaStop: cFrame->onCmdStopSong(); event->accept(); break;
    case Qt::Key_MediaPause: cFrame->onCmdPlayPauseSong(); event->accept(); break;
    case Qt::Key_MediaPrevious: cFrame->onCmdPrevSong(); event->accept(); break;
    case Qt::Key_MediaNext: cFrame->onCmdNextSong(); event->accept(); break;
    default: QMainWindow::keyPressEvent(event); break;
    }
}
