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

#ifndef CENTRALFRAME_H
#define CENTRALFRAME_H

#include "input/input.h"

#include "audio.h"

#include <QThread>
#include <QListView>
#include <QWidget>

class bookmark;
class playlistModel;

class QComboBox;
class QModelIndex;
class QTreeView;
class QFileSystemModel;
class QListView;
class QItemSelectionModel;
class QPushButton;
class QSortFilterProxyModel;

/*****************************************************************/

class loadThread : public QThread
{
    Q_OBJECT

private:
    input *iBackend;
    QString fileName;

protected:
    void run() override
    {
        if (!iBackend->open(fileName))
        {
            utils::delPtr(iBackend);
        }
        emit loaded(iBackend);
    }

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

    input* getInput() const { return _input; }
    void setOpts();

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
    void setInfo(const metaData*);
    void songUpdated(const QString&);

public slots:
    void onCmdPlayPauseSong();
    void onCmdStopSong();
    void onCmdPrevSong();
    void onCmdNextSong();
    void onCmdPlEdit(bool);
    void onCmdPlSave();

    void setFile(const QString& file, const bool play);

    void sortAsc();
    void sortDesc();
    void shuffle();

private slots:
    void onDirSelected(const QModelIndex&);
    void onHome();
    void onCmdCurrentDir();
    void onCmdFiletype(int val);
    void setBackend(int val, int refresh);
    void gotoDir(const QString &dir);
    void onCmdSongLoaded(input* res);
    void onCmdSongSelected(const QModelIndex& currentRow);
    void preloadSong();
    void songEnded();
    void onUpdateTime();
    void onRgtClkDirList(const QPoint& pos);
    void onRgtClkPlayList(const QPoint& pos);

    void onCmdAdd();
    void onCmdBmAdd();
    void updateSongs();

    void scroll(const QString &);

private:
    void load(const QString& filename);
    void onCmdChangeSong(dir_t);
    void setDir(const QModelIndex& index);

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
    QSortFilterProxyModel *_proxyModel;
    QListView *_playlist;
    QComboBox *_fileTypes;
    bookmark *_bookmarkList;
    QPushButton *_editMode;
};

#endif
