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

#ifndef CENTRALFRAME_H
#define CENTRALFRAME_H

#include "player.h"

#include <QThread>
#include <QWidget>

class bookmark;
class playlist;
class playlistModel;
class proxymodel;

class QComboBox;
class QModelIndex;
class QTreeView;
class QFileSystemModel;
class QItemSelectionModel;
class QPushButton;
class QSlider;

/*****************************************************************/

class loadThread : public QThread
{
    Q_OBJECT

private:
    QString fileName;

protected:
    void run();

signals:
    void loaded(input* res);

public:
    loadThread(QString name) :
        QThread(),
        fileName(name)
   {}
};

/*****************************************************************/

class centralFrame : public QWidget
{
    Q_OBJECT

public:
    centralFrame(player* p, QWidget *parent = 0);
    ~centralFrame();

    void onSettingsChanged();

    void setPlayMode(bool mode) { m_playMode = mode; }
    bool getPlayMode() const { return m_playMode; }

    void init();

signals:
    void setDisplay();
    void clearDisplay(const QString&);
    void updateTime(int);
    void updateSlider(int);
    void songUpdated(const QString&);

public slots:
    void onCmdPlayPauseSong();
    void onCmdPrevSong();
    void onCmdNextSong();

    void setFile(const QString& file, const bool play);

private:
    void onCmdPlEdit(bool);
    void onCmdPlSave();

    void onDirSelected(const QModelIndex&);
    void onHome(QAction* action);
    void onCmdCurrentDir();
    void onCmdSongLoaded(input* res);
    void onCmdSongPreLoaded(input* res);
    void onCmdSongSelected(const QModelIndex& currentRow);
    void onPreloadSong();
    void onSongEnded();
    void onUpdateTime();
    void onRgtClkDirList(const QPoint& pos);
    void onRgtClkPlayList(const QPoint& pos);

    void updateSongs();

    void createHomeMenu();
    void load(const QString& filename, bool preload=false);
    void onCmdChangeSong(dir_t);
    void setDir(const QString& index);
    QString getFilter() const;
    QStringList getPattern() const;
    void changeState();

    // Is the song currently loaded from the selected directory?
    bool isPlaylistDirSelected();

private:
    player* m_player;

    // Play mode: true=playlist / false=single song
    bool m_playMode;

    QFileSystemModel *m_fsm;
    QTreeView *m_dirlist;
    playlistModel *m_playlistModel;
    proxymodel *m_proxyModel;
    playlist *m_playlist;
    bookmark *m_bookmarkList;
    QPushButton *m_editMode;
    QPushButton *m_home;
    QSlider *m_slider;
};

#endif
