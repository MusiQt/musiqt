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

#include "audio.h"

#include <QThread>
#include <QWidget>

class bookmark;
class input;
class metaData;
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
    input *iBackend;
    QString fileName;

protected:
    void run();

signals:
    void loaded(input* res);

public:
    loadThread(input* i, QString name) :
        QThread(),
        iBackend(i),
        fileName(name)
   {}
};

/*****************************************************************/

class centralFrame : public QWidget
{
    Q_OBJECT

public:
    enum class dir_t
    {
        ID_PREV,
        ID_NEXT
    };

public:
    centralFrame(QWidget *parent = 0);
    ~centralFrame();

    const metaData* getMetaData() const;

    void onSettingsChanged();

    void setPlayMode(bool mode) { playMode = mode; }
    bool getPlayMode() const { return playMode; }

    int volume() const { return _audio->volume(); }
    void setVolume(int vol) { _audio->volume(vol); }

    void changeSubtune(dir_t dir);

    void init();

signals:
    void stateChanged(state_t state);
    void setDisplay(input*);
    void clearDisplay(bool);
    void updateTime(int);
    void updateSlider(int);
    void setInfo(const metaData*);
    void songUpdated(const QString&);

public slots:
    void onCmdPlayPauseSong();
    void onCmdStopSong();
    void onCmdPrevSong();
    void onCmdNextSong();

    void setFile(const QString& file, const bool play);

private:
    void onCmdPlEdit(bool);
    void onCmdPlSave();

    void onDirSelected(const QModelIndex&);
    void onHome(QAction* action);
    void onCmdCurrentDir();
    void gotoDir(const QString &dir);
    void onCmdSongLoaded(input* res);
    void onCmdSongSelected(const QModelIndex& currentRow);
    void preloadSong();
    void songEnded();
    void onUpdateTime();
    void onRgtClkDirList(const QPoint& pos);
    void onRgtClkPlayList(const QPoint& pos);

    void updateSongs();
    void onSeek();

    void createHomeMenu();
    void load(const QString& filename);
    void onCmdChangeSong(dir_t);
    void setDir(const QModelIndex& index);
    QString getFilter() const;
    QStringList getPattern() const;
    void changeState();

private:
    input *_input;
    input *_preload;
    audio *_audio;
    bool playing;
    bool playMode;
    QString preloaded;
    QString playDir;

    QFileSystemModel *fsm;
    QTreeView *_dirlist;
    playlistModel *_playlistModel;
    proxymodel *_proxyModel;
    playlist *_playlist;
    bookmark *_bookmarkList;
    QPushButton *_editMode;
    QPushButton *m_home;
    QSlider *m_slider;
};

#endif
