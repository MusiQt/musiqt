/*
 *  Copyright (C) 2013-2023 Leandro Nini
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
#include "player.h"

#include <QStatusBar>
#include <QAction>
#include <QDir>
#include <QToolButton>
#include <QFileInfo>
#include <QLayout>
#include <QDial>
#include <QMenu>
#include <QToolBar>
#include <QKeyEvent>
#include <QSignalBlocker>

#ifdef _WIN32
#  include <windows.h>
#endif

#include "ledRed.xpm"
#include "ledYellow.xpm"
#include "ledGreen.xpm"

mainWindow::mainWindow(player* p, QWidget *parent) :
    QMainWindow(parent),
    m_player(p),
    m_infoDialog(nullptr)
{
    connect(m_player, &player::subtunechanged, this, &mainWindow::setDisplay);
    connect(m_player, &player::stateChanged,   this, &mainWindow::setPlayButton);
    connect(m_player, &player::audioError,     this, &mainWindow::showError);

    // Read settings
    SETTINGS->load(m_settings);

    createActions();

    //setAttribute(Qt::WA_AlwaysShowToolTips);
    statusBar()->showMessage(PACKAGE_STRING);
    connect(statusBar(), &QStatusBar::messageChanged,
        [this](const QString &message)
        {
            if (message.isNull())
                statusBar()->showMessage(PACKAGE_STRING);
        }
    );

    QToolButton *about = new QToolButton();
    about->setAutoRaise(true);
    about->setDefaultAction(m_aboutAction);
    statusBar()->addPermanentWidget(about);

    QToolButton *quit = new QToolButton();
    quit->setAutoRaise(true);
    quit->setDefaultAction(m_quitAction);
    statusBar()->addPermanentWidget(quit);

    m_cFrame = new centralFrame(m_player, this);
    setCentralWidget(m_cFrame);

    connect(m_playAction, &QAction::triggered, m_cFrame, &centralFrame::onCmdPlayPauseSong);
    connect(m_stopAction, &QAction::triggered,
        [this]() { m_player->stop(); }
    );
    connect(m_prevAction, &QAction::triggered, m_cFrame, &centralFrame::onCmdPrevSong);
    connect(m_nextAction, &QAction::triggered, m_cFrame, &centralFrame::onCmdNextSong);

    connect(m_cFrame, &centralFrame::setDisplay,   this, &mainWindow::setDisplay);
    connect(m_cFrame, &centralFrame::clearDisplay, this, &mainWindow::clearDisplay);

    addToolBar(createControlBar());
    addToolBarBreak();
    addToolBar(createSecondaryBar());  
    addToolBar(Qt::BottomToolBarArea, createInfoBar());

    setWindowTitle(PACKAGE_NAME);
    setWindowIcon(GET_ICON(icon_logo32));

    if (QSystemTrayIcon::isSystemTrayAvailable())
    {
        createTrayIcon();

        connect(m_trayIcon, &QSystemTrayIcon::activated, this, &mainWindow::iconActivated);

        m_trayIcon->setIcon(GET_ICON(icon_logo32));
        m_trayIcon->setToolTip(PACKAGE_STRING);
        m_trayIcon->show();
    }

    qApp->setStyleSheet("QMainWindow > QPushButton,QToolButton { margin:0; padding:0; }");

    QPoint pos = m_settings.value(config::GENERAL_POS).toPoint();
    QSize size = m_settings.value(config::GENERAL_SIZE).toSize();

    resize(size);
    move(pos);
}

mainWindow::~mainWindow()
{
    // Save Settings
    SETTINGS->save(m_settings);

    if (!centralWidget()->isHidden())
    {
        m_settings.setValue(config::GENERAL_SIZE, size());
    }
    m_settings.setValue(config::GENERAL_POS, pos());
    m_settings.setValue(config::GENERAL_PLMODE, m_cFrame->getPlayMode());

    m_settings.setValue(config::GENERAL_FILE, m_player->loadedSong());
}

void mainWindow::init(const char* arg)
{
    m_cFrame->init();

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
        onMessage(message);
    }
    else
    {
        m_player->load(m_settings.value(config::GENERAL_FILE).toString());
    }
}

void mainWindow::onMessage(QString msg)
{
    m_player->loadAndPlay(QDir(msg).absolutePath());
}

void mainWindow::notify(const QString& title, const QString &text)
{
    m_trayIcon->showMessage(title, text);
}

QToolBar *mainWindow::createControlBar()
{
    m_timeDisplay = new timeDisplay(this);

    QToolBar *controlBar = new QToolBar("controlBar", this);
    controlBar->addAction(m_prevAction);
    controlBar->addAction(m_stopAction);
    controlBar->addAction(m_playAction);
    controlBar->addAction(m_nextAction);

    controlBar->addWidget(m_timeDisplay);

    QAction *act = controlBar->addAction(GET_ICON(icon_viewcompact), tr("Compact"));
    act->setToolTip(tr("Compact"));
    act->setStatusTip(tr("Switch to compact view"));
    connect(act, &QAction::triggered, this, &mainWindow::onCompact);

    act = controlBar->addAction(GET_ICON(icon_preferencesdesktop), tr("Settings"));
    act->setToolTip(tr("Settings"));
    act->setStatusTip(tr("Program settings"));
    connect(act, &QAction::triggered, this, &mainWindow::onConfig);

    controlBar->setMovable(false);

    connect(m_cFrame, &centralFrame::updateTime,   m_timeDisplay, &timeDisplay::setTime);

    return controlBar;
}

QToolBar *mainWindow::createSecondaryBar()
{
    QAction *prevtune = new QAction(GET_ICON(icon_goprevious), tr("Previous"), this);
    prevtune->setToolTip(tr("Previous"));
    prevtune->setStatusTip(tr("Go to previous subtune"));
    connect(prevtune, &QAction::triggered,
        [this]() { m_player->changeSubtune(dir_t::ID_PREV); }
    );

    QAction *nexttune = new QAction(GET_ICON(icon_gonext), tr("Next"), this);
    nexttune->setToolTip(tr("Next"));
    nexttune->setStatusTip(tr("Go to next subtune"));
    connect(nexttune, &QAction::triggered,
        [this]() { m_player->changeSubtune(dir_t::ID_NEXT); }
    );

    QAction *playlist = new QAction(GET_ICON(icon_playlist), tr("Playlist"), this);
    const bool playMode = m_settings.value(config::GENERAL_PLMODE, true).toBool();
    setPlayMode(playlist, playMode);
    connect(playlist, &QAction::triggered,
        [this, playlist]() { setPlayMode(playlist, !m_cFrame->getPlayMode()); }
    );
#ifdef HAVE_LASTFM
    QPixmap pixmapOn(icon_lastfm);
    QPixmap pixmapOff(QPixmap::fromImage(pixmapOn.toImage().convertToFormat(QImage::Format_Grayscale8)));
    QIcon icon;
    icon.addPixmap(pixmapOn, QIcon::Normal, QIcon::On);
    icon.addPixmap(pixmapOff, QIcon::Normal, QIcon::Off);
    QAction *scrobble = new QAction(icon, tr("Scrobble"), this);
    scrobble->setCheckable(true);
    scrobble->setStatusTip(tr("Enable/disable scrobbling"));
    const bool scrobbling = m_settings.value(config::LASTFM_SCROBBLING, true).toBool();
    scrobble->setChecked(scrobbling);
    connect(scrobble, &QAction::triggered, this, &mainWindow::setScrobbling);
#endif
    m_subtunes = new QLabel(this);
    m_subtunes->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    m_subtunes->setText("00/00");
    m_subtunes->setStatusTip(tr("Subtunes"));

    QLabel *songs = new QLabel(this);
    songs->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    songs->setText("00/00");
    songs->setStatusTip(tr("Songs"));
    connect(m_cFrame, &centralFrame::songUpdated, songs, &QLabel::setText);

    m_statusLed = new QLabel(this);
    m_statusLed->setPixmap(QPixmap(ledRed));
    m_statusLed->setStatusTip(tr("Stopped"));
    m_statusLed->setStyleSheet("QLabel {margin: 0 5;}");

    QWidget* empty = new QWidget(this);
    empty->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    QDial *volume = new QDial(this);
    volume->setRange(0, 100);
    volume->setStatusTip(tr("Volume"));
    volume->setValue(m_player->getVolume());
    connect(volume, &QDial::valueChanged, m_player, &player::setVolume);
    connect(m_player, &player::volumeChanged,
        [this, volume]()
        {
            const QSignalBlocker blocker(volume);
            volume->setValue(m_player->getVolume());
        }
    );

    QToolBar *secondaryBar = new QToolBar("secondaryBar", this);
    secondaryBar->addAction(prevtune);
    secondaryBar->addWidget(m_subtunes);
    secondaryBar->addAction(nexttune);
    secondaryBar->addWidget(m_statusLed);
    
    secondaryBar->addWidget(songs);
    secondaryBar->addAction(playlist);
#ifdef HAVE_LASTFM
    secondaryBar->addAction(scrobble);
#endif
    secondaryBar->addWidget(empty);
    secondaryBar->addWidget(volume);

    const int height = secondaryBar->height();
    volume->setMaximumSize(height, height);

    secondaryBar->setMovable(false);

    return secondaryBar;
}

QToolBar *mainWindow::createInfoBar()
{
    m_songInfo = new infoLabel(this);
    m_songInfo->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    m_songInfo->setText("--");
    m_songInfo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    m_songTime = new timeLabel(this);

    QToolBar *infoBar = new QToolBar("infoBar", this);
    infoBar->addAction(m_infoAction);
    infoBar->addWidget(m_songInfo);
    infoBar->addWidget(m_songTime);

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
    m_aboutAction = new QAction(GET_ICON(icon_logo16), tr("&About"), this);
    m_aboutAction->setToolTip(tr("About"));
    m_aboutAction->setStatusTip(tr("About"));
    connect(m_aboutAction, &QAction::triggered, this, &mainWindow::onAbout);

    m_prevAction = new QAction(GET_ICON(icon_mediaskipbackward), tr("P&revious"), this);
    m_prevAction->setToolTip(tr("Previous"));
    m_prevAction->setStatusTip(tr("Go to previous song"));

    m_nextAction = new QAction(GET_ICON(icon_mediaskipforward), tr("&Next"), this);
    m_nextAction->setToolTip(tr("Next"));
    m_nextAction->setStatusTip(tr("Go to next song"));

    m_playAction = new QAction(GET_ICON(icon_mediaplaybackstart), tr("&Play"), this);
    m_playAction->setToolTip(tr("Play"));
    m_playAction->setStatusTip(tr("Play song"));

    m_stopAction = new QAction(GET_ICON(icon_mediaplaybackstop), tr("&Stop"), this);
    m_stopAction->setToolTip(tr("Stop"));
    m_stopAction->setStatusTip(tr("Stop"));

    m_infoAction = new QAction(GET_ICON(icon_helpabout), tr("&Info"), this);
    m_infoAction->setToolTip(tr("Info"));
    m_infoAction->setStatusTip(tr("Song info"));
    connect(m_infoAction, &QAction::triggered, this, &mainWindow::onInfo);

    m_quitAction = new QAction(GET_ICON(icon_applicationexit), tr("&Quit"), this);
    m_quitAction->setToolTip(tr("Exit"));
    m_quitAction->setStatusTip(tr("Exit"));
    connect(m_quitAction, &QAction::triggered, qApp, &QApplication::quit);
}

void mainWindow::createTrayIcon()
{
    QMenu* m_trayIconMenu = new QMenu(this);
    m_trayStatus = new QAction(m_trayIconMenu);
    m_trayStatus->setText(tr("Stopped"));
    m_trayStatus->setIcon(QPixmap(ledRed));

    m_trayIconMenu->addAction(m_trayStatus);
    m_trayIconMenu->addSeparator();
    m_trayIconMenu->addAction(m_playAction);
    m_trayIconMenu->addAction(m_stopAction);
    m_trayIconMenu->addAction(m_prevAction);
    m_trayIconMenu->addAction(m_nextAction);
    m_trayIconMenu->addAction(m_infoAction);
    m_trayIconMenu->addSeparator();
    m_trayIconMenu->addAction(m_quitAction);

    m_trayIcon = new QSystemTrayIcon(this);
    m_trayIcon->setContextMenu(m_trayIconMenu);
}

void mainWindow::onCompact()
{
    QAction* act = (QAction*)sender();
    if (centralWidget()->isHidden())
    {
        act->setIcon(GET_ICON(icon_viewcompact));
        act->setToolTip(tr("UI mode"));
        act->setStatusTip(tr("Switch to compact view"));

        centralWidget()->show();
        statusBar()->setSizeGripEnabled(true);
        setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
        resize(m_windowSize);
    }
    else
    {
        act->setIcon(GET_ICON(icon_viewfull));
        act->setToolTip(tr("UI mode"));
        act->setStatusTip(tr("Switch to full view"));

        if (isMaximized())
            return;
        m_windowSize = size();
        centralWidget()->hide();
        statusBar()->setSizeGripEnabled(false);
        adjustSize();
        setFixedSize(size());
    }
}

void mainWindow::onAbout()
{
    aboutDialog *about = new aboutDialog(this);
    about->open();
    connect(about, &aboutDialog::finished,
        [about, this]()
        {
            about->deleteLater();
        }
    );
}

void mainWindow::onConfig()
{
    settingsWindow *config = new settingsWindow(this);
    config->open();
    connect(config, &settingsWindow::finished,
        [config, this](int result)
        {
            switch (result)
            {
            case QDialog::Accepted:
                SETTINGS->save(m_settings);
                m_cFrame->onSettingsChanged();
                break;
            case QDialog::Rejected:
                SETTINGS->load(m_settings);
                break;
            }
            config->deleteLater();
        }
    );
}

void mainWindow::onInfo()
{
    if (m_infoDialog != nullptr)
        return;

    m_infoDialog = new infoDialog(this);
    m_infoDialog->setInfo(m_player->getMetaData());
    m_infoDialog->setAttribute(Qt::WA_QuitOnClose, false);
    connect(m_infoDialog, &infoDialog::finished,
        [this]()
        {
            qDebug("closing info dialog");
            m_infoDialog->deleteLater();
            m_infoDialog = nullptr;
        }
    );
    m_infoDialog->show();
}

void mainWindow::setPlayButton()
{
    state_t state = m_player->state();
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

        m_playAction->setIcon(GET_ICON(icon));
        m_playAction->setToolTip(label);
        m_playAction->setStatusTip(label);
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
        default:
#ifdef __GNUC__
            __builtin_unreachable();
#elif defined(_MSC_VER)
            __assume(0);
#endif
        }

        m_trayStatus->setIcon(QPixmap(icon));
        m_trayStatus->setText(label);
        m_statusLed->setPixmap(QPixmap(icon));
        m_statusLed->setStatusTip(label);
    }
}

void mainWindow::setDisplay()
{
    m_timeDisplay->setTime(0);
    m_songTime->setTime(m_player->songDuration() / 1000);

    m_subtunes->setText(QString("%1/%2").arg(m_player->subtune()).arg(m_player->subtunes()));

    const metaData* data = m_player->getMetaData();
    QString songTitle = data->getInfo(metaData::TITLE);
    if (songTitle.isEmpty())
        songTitle = QFileInfo(m_player->loadedSong()).fileName();

    setWindowTitle(songTitle);

    QString artist = data->getInfo(metaData::ARTIST);
    if (!artist.isEmpty())
        artist.append('\n');

    m_trayIcon->setToolTip(artist+songTitle);

    m_songInfo->setInfo(data);

    if (m_infoDialog != nullptr)
        m_infoDialog->setInfo(data);
}

void mainWindow::clearDisplay(const QString& text)
{
    m_timeDisplay->reset();
    m_songTime->reset();

    setWindowTitle(QString(PACKAGE_STRING));
    m_trayIcon->setToolTip(PACKAGE_STRING);

    m_songInfo->setText(text);
    m_songInfo->setToolTip(QString());

    if (m_infoDialog != nullptr)
        m_infoDialog->setInfo(m_player->getMetaData());
}

void mainWindow::setPlayMode(QAction *action, bool mode)
{
    m_cFrame->setPlayMode(mode);
    if (mode)
    {
        action->setToolTip(tr("Play mode"));
        action->setStatusTip(tr("Switch to song mode"));
        action->setIcon(GET_ICON(icon_playlist));
    }
    else
    {
        action->setToolTip(tr("Play mode"));
        action->setStatusTip(tr("Switch to playlist mode"));
        action->setIcon(GET_ICON(icon_song));
    }
}

void mainWindow::keyPressEvent(QKeyEvent *event)
{
    switch (event->key())
    {
    case Qt::Key_MediaPlay:             m_player->play();                               event->accept(); break;
    case Qt::Key_MediaStop:             m_player->stop();                               event->accept(); break;
    case Qt::Key_MediaPause:            m_player->pause();                              event->accept(); break;
    case Qt::Key_MediaTogglePlayPause:  m_cFrame->onCmdPlayPauseSong();                 event->accept(); break;
    case Qt::Key_MediaPrevious:         m_cFrame->onCmdPrevSong();                      event->accept(); break;
    case Qt::Key_MediaNext:             m_cFrame->onCmdNextSong();                      event->accept(); break;
    case Qt::Key_VolumeUp:              m_player->setVolume(m_player->getVolume()+5);   event->accept(); break;
    case Qt::Key_VolumeDown:            m_player->setVolume(m_player->getVolume()-5);   event->accept(); break;
    case Qt::Key_VolumeMute:            m_player->setVolume(0);                         event->accept(); break;
    default: QMainWindow::keyPressEvent(event); break;
    }
}

void mainWindow::showError(const QString& msg)
{
    m_trayIcon->showMessage(tr("Error!"), msg);
    //statusBar()->showMessage(msg);
}
