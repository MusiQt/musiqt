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

#include "centralFrame.h"

#include "iconFactory.h"
#include "bookmark.h"
#include "playlist.h"
#include "input/inputFactory.h"
#include "settings.h"
#include "trackListFactory.h"
#include "xdg.h"

#include <QApplication>
#include <QButtonGroup>
#include <QComboBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QFileInfo>
#include <QFileSystemModel>
#include <QItemSelectionModel>
#include <QListWidget>
#include <QListView>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QSignalMapper>
#include <QStackedWidget>
#include <QTimer>
#include <QTreeView>
#include <QWidgetAction>
#include <QDebug>

centralFrame::centralFrame(QWidget *parent) :
    QWidget(parent),
    _input(nullptr),
    _preload(nullptr),
    _audio(new audio),
    playing(false),
    preloaded(QString::null),
    playDir(QString::null)
{
    connect(_audio, SIGNAL(outputError()), this, SLOT(onCmdStopSong()));
    connect(_audio, SIGNAL(updateTime()), this, SLOT(onUpdateTime()));
    connect(_audio, SIGNAL(songEnded()), this, SLOT(songEnded()));
    connect(_audio, SIGNAL(preloadSong()), this, SLOT(preloadSong()));

    // dir view
    fsm = new QFileSystemModel(this);
    fsm->setFilter(QDir::AllDirs|QDir::Drives|QDir::NoDotAndDotDot|QDir::Files);
    fsm->setRootPath(QDir::rootPath());
    fsm->setNameFilterDisables(false);
    fsm->setNameFilters(TFACTORY->plExt());

    _dirlist = new QTreeView(this);
    _dirlist->setModel(fsm);
    _dirlist->hideColumn(3);
    _dirlist->hideColumn(2);
    _dirlist->hideColumn(1);
    _dirlist->setHeaderHidden(true);
    _dirlist->setUniformRowHeights(true);
    _dirlist->setDragEnabled(true);
    _dirlist->setContextMenuPolicy(Qt::CustomContextMenu);
    QItemSelectionModel* selection = _dirlist->selectionModel();

    setProperty("AutoBackend", QVariant(true));

    connect(selection, SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)),
            this, SLOT(onDirSelected(const QModelIndex&)));
    connect(_dirlist, SIGNAL(customContextMenuRequested(const QPoint&)),
            this, SLOT(onRgtClkDirList(const QPoint&)));

    // _playlist
    _playlist = new playlist(this);
    _playlist->setAlternatingRowColors(true);
    _playlist->setUniformItemSizes(true);

    connect(_playlist, SIGNAL(currentRowChanged(int)), this, SLOT(onCmdSongSelected(int)));
    connect(_playlist, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(onCmdPlayPauseSong()));
    connect(_playlist, SIGNAL(changed()), this, SLOT(updateSongs()));

    _bookmarkList = new bookmark(this);
    _bookmarkList->setAlternatingRowColors(true);
    connect(_bookmarkList, SIGNAL(currentTextChanged(const QString&)), this, SLOT(gotoDir(const QString&)));

    // left view
    QStackedWidget *stackedWidget = new QStackedWidget(this);
    stackedWidget->addWidget(_playlist);
    stackedWidget->addWidget(_bookmarkList);
    QButtonGroup *buttonGroup = new QButtonGroup(this);
    connect(buttonGroup, SIGNAL(buttonClicked(int)), stackedWidget, SLOT(setCurrentIndex(int)));

    QWidget *w = new QWidget(this);
    {
        QHBoxLayout *buttons = new QHBoxLayout(w);
        buttons->setContentsMargins(0,0,0,0);
        QPushButton *b1 = new QPushButton(GET_ICON(icon_playlist), tr("Playlist"), this);
        b1->setCheckable(true);
        b1->setAutoExclusive(true);
        b1->setChecked(true);
        buttonGroup->addButton(b1, 0);
        buttons->addWidget(b1);
        QPushButton *b2 = new QPushButton(GET_ICON(icon_bookmark), tr("Bookmarks"), this);
        b2->setCheckable(true);
        b2->setAutoExclusive(true);
        buttonGroup->addButton(b2, 1);
        buttons->addWidget(b2);
        buttons->setSpacing(0);
    }

    //
    QWidget *wButtons = new QWidget(this);
    {
        QHBoxLayout *buttons = new QHBoxLayout(wButtons);
        buttons->setContentsMargins(0,0,0,0);
        _fileTypes = new QComboBox(this);
        _fileTypes->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        buttons->addWidget(_fileTypes);
        _editMode = new QPushButton(this);
        _editMode->setIcon(GET_ICON(icon_editlist));
        _editMode->setToolTip(tr("Edit"));;
        _editMode->setStatusTip(tr("Edit playlist"));
        _editMode->setCheckable(true);
        buttons->addWidget(_editMode);
        connect(_editMode, SIGNAL(clicked(bool)), this, SLOT(onCmdPlEdit(bool)));
        QPushButton *b1 = new QPushButton(this);
        b1->setIcon(GET_ICON(icon_documentsave));
        b1->setToolTip(tr("Save"));
        b1->setStatusTip(tr("Save playlist"));
        connect(b1, SIGNAL(clicked()), this, SLOT(onCmdPlSave()));
        buttons->addWidget(b1);
        b1 = new QPushButton(this);
        b1->setIcon(GET_ICON(icon_currentplaylist));
        b1->setToolTip(tr("Current playlist"));
        b1->setStatusTip(tr("Return to current playlist"));
        connect(b1, SIGNAL(clicked()), this, SLOT(onCmdPlBack()));
        buttons->addWidget(b1);
        b1 = new QPushButton(this);
        b1->setIcon(GET_ICON(icon_gohome));
        b1->setToolTip(tr("Home"));
        b1->setStatusTip(tr("Home"));
        connect(b1, SIGNAL(clicked()), this, SLOT(onHome()));
        buttons->addWidget(b1);
        buttons->setSpacing(0);

        for (int i=0; i<IFACTORY->num(); i++)
            _fileTypes->addItem(IFACTORY->name(i));

        _fileTypes->setMaxVisibleItems((_fileTypes->count() > 5) ? 5 : _fileTypes->count());
        connect(_fileTypes, SIGNAL(currentIndexChanged(int)), this, SLOT(onCmdFiletype(int)));
    }

    // Left view - file list/bookmarks
    QWidget *w2 = new QWidget(this);
    QVBoxLayout *vbox = new QVBoxLayout(w2);
    vbox->setContentsMargins(0,0,0,0);
    vbox->addWidget(w);
    vbox->addWidget(stackedWidget);
    vbox->addWidget(wButtons);
    vbox->setSpacing(0);

    QHBoxLayout *hbox = new QHBoxLayout(this);
    hbox->setContentsMargins(0,0,0,0);
    hbox->addWidget(w2);
    hbox->addWidget(_dirlist);

    setBackend(0, 0);
}

centralFrame::~centralFrame()
{
    _bookmarkList->save();

    QSettings settings; // FIXME
    QString file = _input->songLoaded();
    settings.setValue("General Settings/file", file);

    delete _audio;
    delete _input;
}

void centralFrame::onDirSelected(const QModelIndex& idx)
{
    bool autoBk = property("AutoBackend").toBool();
    setProperty("AutoBackend", QVariant(true));

    if (_editMode->isChecked())
        return;

    const QString curItem = fsm->fileName(_dirlist->currentIndex());
    if (curItem.isEmpty())
        return;

    qDebug() << "Set dir " << curItem;

    const QString mPath = fsm->fileInfo(idx).absoluteFilePath();
    qDebug() << mPath;
    int items = _playlist->load(mPath);
    if (items == 0)
    {
        _playlist->clear();
        return;
    }
    items = (_input != nullptr) ? _playlist->filter(_input->ext()) : 0;

    if (items == 0)
    {
        if (playing || !SETTINGS->autoBk() || !autoBk)
            return;
        int bk = 0;
        do {
            if (bk != _fileTypes->currentIndex())
            {
                input* i = IFACTORY->get(bk);
                items = _playlist->filter(i->ext());
                delete i;
            }
        } while ((++bk < _fileTypes->count()) && (items == 0));
        if (items != 0)
            setBackend(bk-1, 0);
        else
            return;
    }

    //_playlist->sortItems();
    _playlist->setCurrentRow(-1);

    const QString fileName = _dirlist->property("UserData").toString();
    if (!fileName.isEmpty())
    {
        QList<QListWidgetItem *> items = _playlist->findItems(fileName, Qt::MatchExactly|Qt::MatchCaseSensitive);
        const int i = items.empty() ? -1 : _playlist->row(items.at(0));

        _playlist->setCurrentRow((i < 0) ? 0 : i);
        _dirlist->setProperty("UserData", QVariant(QString::null));
    }
    else
    {
        if (!curItem.compare(playDir))
        {
            QFileInfo fileInfo(_input->songLoaded());
            QList<QListWidgetItem *> items = _playlist->findItems(fileInfo.completeBaseName(),
                                                                  Qt::MatchExactly|Qt::MatchCaseSensitive);
            if (!items.empty())
            {
                QListWidgetItem *item = items.at(0);
                _playlist->setCurrentRow(_playlist->row(item));
                _playlist->scrollToItem(item);
            }
        }
        else if (!playing)
            _playlist->setCurrentRow(0);
    }
}

void centralFrame::onHome()
{
    if (_input != nullptr)
    {
        const QString musicDir = _input->getMusicDir();
        QString home = musicDir.isEmpty() ? xdg::getMusicDir() : musicDir;
        qDebug() << "home dir: " << home;
        setDir(fsm->index(home));
    }
}

void centralFrame::onCmdPlBack()
{
    QFileInfo fileInfo(_input->songLoaded());
    gotoDir(fileInfo.absolutePath());

     QList<QListWidgetItem*> items = _playlist->findItems(fileInfo.completeBaseName(), Qt::MatchExactly|Qt::MatchCaseSensitive);
            const int idx = items.empty() ? -1 : _playlist->row(items.at(0));
    _playlist->setCurrentRow(idx);
}

void centralFrame::gotoDir(const QString &dir)
{
    qDebug() << "gotoDir: " << dir;
    if (!dir.isNull())
        setDir(fsm->index(dir));
}
 
void centralFrame::setFile(const QString& file, const bool play)
{
    qDebug() << "setFile " << file;

    int curItem = _playlist->currentRow();

    QFileInfo qfile(file);
    // Check if requested file/directory exists
    if (!qfile.exists())
    {
        if (curItem >= 0) // ???
            onHome();
        return;
    }

    QList<QListWidgetItem*> items = _playlist->findItems(file, Qt::MatchExactly|Qt::MatchCaseSensitive);
    QListWidgetItem *selectedItem = items.empty() ? nullptr : items.at(0);
    const bool selected = items.empty() ? false : items.at(0)->isSelected();

    // Check if requested song is already playing
    if (playing && selected)
        return;

    int val = selectedItem != nullptr ? _playlist->row(selectedItem) : -1;

    QString currentDir = fsm->fileName(_dirlist->currentIndex());
    const bool dirSelected = !currentDir.compare(qfile.dir().absolutePath());

    if (qfile.isDir() || (TFACTORY->plExt().indexOf(qfile.suffix().toLower()) >= 0))
    {
        if (dirSelected)
        {
            if (play && !playing)
            {
                // emit play();
                onCmdPlayPauseSong();
            }
            return;
        }
        else
        {
            _dirlist->setProperty("UserData", QVariant(QString::null));
            setDir(fsm->index(file));
            playMode = true; // TODO sync GUI
        }
        goto done;
    }

    {
        const QString fName = qfile.fileName();

        if (!fName.isEmpty())
        {
            QRegExp regexp(_input->ext().join("|"));
            regexp.setPatternSyntax(QRegExp::Wildcard);
            if ((_input != nullptr)
                && regexp.exactMatch(fName))
            {
                goto ok;
            }

            for (int i=0; i<IFACTORY->num(); i++)
            {
                qDebug() << "check factory " << i;
                if (IFACTORY->supports(i, fName))
                {
                    //emit stop();

                    setBackend(i, dirSelected ? 1 : 0);
                    items = _playlist->findItems(file, Qt::MatchExactly|Qt::MatchCaseSensitive);
                    val = items.empty() ? -1 : _playlist->row(items.at(0));
                    curItem = -1;
                    goto ok;
                }
            }

            // file is not supported, ignore message
            return;
        }
    }
ok:
    if (dirSelected)
    {
        if (val >= 0)
        {
            if (val == curItem)
            {
                //emit play();
                onCmdPlayPauseSong();
            }
            else
                _playlist->setCurrentRow(val);
        }
    } else {
        _dirlist->setProperty("UserData", QVariant(QFileInfo(file).completeBaseName()));
        setDir(fsm->index(qfile.dir().absolutePath()));
    }

done:
    playing = play;
}

void centralFrame::onCmdPlayPauseSong()
{
    if (_audio->state() != state_t::STOP)
    {
        _audio->pause();
    }
    else if (_audio->play(_input))
    {
        playing = true;
        playDir = fsm->fileName(_dirlist->currentIndex());
    }

    _fileTypes->setEnabled(false);
    emit stateChanged(_audio->state());
}

void centralFrame::onCmdStopSong()
{
    if (_audio->stop())
    {
        playing = false;
        _fileTypes->setEnabled(true);
        emit updateTime(0);
        emit stateChanged(_audio->state());
        QModelIndex curr = _dirlist->currentIndex();
        if (playDir.compare(fsm->fileName(curr)))
        {
            setProperty("AutoBackend", QVariant(true));
            onDirSelected(curr);
        }
        playDir = QString::null;
    }
}

void centralFrame::onCmdPrevSong()
{
    onCmdChangeSong(ID_PREV);
}

void centralFrame::onCmdNextSong()
{
    onCmdChangeSong(ID_NEXT);
}

void centralFrame::onCmdChangeSong(dir_t dir)
{
    if (playing && playDir.compare(fsm->fileName(_dirlist->currentIndex())))
        return;

    int idx = _playlist->currentRow();

    switch (dir)
    {
    case ID_PREV:
            idx--;
            qDebug("Previous");
            break;
    case ID_NEXT:
            idx++;
            qDebug("Next");
            break;
    }

    if ((idx >= 0) && (idx < _playlist->count()))
    {
        if (!preloaded.isEmpty())
        {
            _preload->close();
            preloaded = QString::null;
        }
        _playlist->setCurrentRow(idx);
    }
}

void centralFrame::setBackend(int val, int refresh)
{
    qDebug() << "setBackend " << val;
    if ((val < 0) || val>_fileTypes->count())
        return;

    _fileTypes->setCurrentIndex(val);

    if (_input != nullptr)
    {
        _bookmarkList->save();
        emit clearDisplay(false);
        delete _input;
        delete _preload;
    }

    _input = IFACTORY->get(val);
    _preload = nullptr;
    _playlist->filter(_input->ext());

    _bookmarkList->load(IFACTORY->name(val));

    if (refresh != 0)
    {
        QModelIndex selected = _dirlist->currentIndex();
        qDebug() << "selected " << selected;
        if (selected.isValid())
        {
            setProperty("AutoBackend", QVariant((refresh != 2)));
            onDirSelected(selected);
            //_dirlist->selectionModel()->select(selected, QItemSelectionModel::ClearAndSelect);
        }
    }
}

void centralFrame::load(const QString& filename)
{
    qDebug() << "Loading " << filename;

    _fileTypes->setEnabled(false);

    loadThread* loader = new loadThread(IFACTORY->get(_fileTypes->currentIndex()), filename);
    connect(loader, SIGNAL(loaded(input*)), this, SLOT(onCmdSongLoaded(input*)));
    connect(loader, SIGNAL(finished()), loader, SLOT(deleteLater()));

    loader->start();
}

void centralFrame::onCmdSongLoaded(input* res)
{
    if (!preloaded.isEmpty())
    {
        if ((res != nullptr) && _audio->gapless(res))
        {
            delete _preload;
            _preload = res;

            if (SETTINGS->subtunes())
                _preload->subtune(1);
        }
        else
        {
            delete res;
            preloaded = QString::null;
        }
        return;
    }

    if (res != nullptr)
    {
        input* tmp = _input;
        _input = res;
        delete tmp;

        emit setDisplay(_input);

        if (SETTINGS->subtunes())
            _input->subtune(1);

        if (playing)
        {
            //emit play();
            onCmdPlayPauseSong();
        }
        qDebug() << "Song loaded";
    }
    else
    {
        playing = false;
        playDir = QString::null;
        emit setInfo(nullptr);
        emit stateChanged(_audio->state());
        qWarning() << "Error loading song";
    }

    _fileTypes->setEnabled(true);
    QApplication::restoreOverrideCursor();
}

void centralFrame::onCmdSongSelected(int currentRow)
{
    if (_editMode->isChecked())
        return;

    qDebug() << "onCmdSongSelected " << currentRow;
    if (currentRow < 0)
        return;

    QString songLoaded = _input->songLoaded();
    const QString song = _playlist->getLocation(currentRow);
    if (!songLoaded.isEmpty() && !song.compare(songLoaded))
        return;

    _playlist->scrollToItem( _playlist->item(currentRow));

    if (!preloaded.isEmpty())
    {
        if (preloaded.compare(song))
        {
            preloaded = QString::null;
            utils::delPtr(_preload);
        }
        else
        {
            delete _input;
            _input = _preload;
            _preload = nullptr;
            preloaded = QString::null;
            emit setDisplay(_input);
            return;
        }
    }

    {
        QApplication::setOverrideCursor(Qt::WaitCursor);
        _audio->stop();
        emit clearDisplay(true);
    }

    load(song);
    updateSongs();
}

void centralFrame::onUpdateTime()
{
    // Playing may have stopped while message was in queue
    if (_audio->state() != state_t::STOP)
    {
        emit updateTime(_audio->seconds());
    }
}

void centralFrame::preloadSong()
{
    if (playMode && _input->gapless() && !playDir.compare(fsm->fileName(_dirlist->currentIndex())))
    {
        const int nextSong = _playlist->currentRow()+1;
        if (nextSong < _playlist->count())
        {
            preloaded = _playlist->getLocation(nextSong);
            load(preloaded);
        }
    }
}

void centralFrame::songEnded()
{
    qDebug("centralFrame::songEnded");
    if (SETTINGS->subtunes() && (_input->subtune()<_input->subtunes()))
    {
        changeSubtune(ID_NEXT);
        return;
    }

    if (playMode)
    {
        int item = 0;
        if (!playDir.compare(fsm->fileName(_dirlist->currentIndex())))
        {
            item = _playlist->currentRow() + 1;
        }

        if ((item >= 0) && (item < _playlist->count()))
        {
            _playlist->setCurrentRow(item);
            updateSongs(); 
            return;
        }
    }

    // emit stop();
    onCmdStopSong();
}

void centralFrame::setOpts()
{
    _audio->setOpts();
    _input->saveSettings();
    if (_playlist->count())
    {
        // if settings changes we must reload the song
        _input->close();
        const int curItem = _playlist->currentRow();
        _playlist->setCurrentRow(-1);
        _playlist->setCurrentRow(curItem);
    }
}

void centralFrame::onCmdFiletype(int val)
{
    setBackend(val, 2);
}

void centralFrame::onRgtClkDirList(const QPoint& pos)
{
    qDebug() << "onRgtClkDirList";
    QModelIndex item = _dirlist->indexAt(pos);
    if (!item.isValid())
        return;

    QMenu pane(this);
    QWidgetAction *wa = new QWidgetAction(&pane);
    wa->setDefaultWidget(new QLabel(utils::shrink(fsm->fileName(item))));
    pane.addAction(wa);
    pane.addSeparator();

    if ((_editMode->isChecked()) && (!fsm->isDir(item)))
    {
        QAction *additem = pane.addAction(GET_ICON(icon_listadd), tr("Add to playlist"), this, SLOT(onCmdAdd()));
        additem->setStatusTip(tr("Add song to the playlist"));
    }
    else
    {
        QAction *additem = pane.addAction(GET_ICON(icon_bookmark), tr("Add to bookmarks"), this, SLOT(onCmdBmAdd()));
        additem->setStatusTip(tr("Add directory to the bookmarks"));
    }

    //setUserData((void*)item);

    pane.exec(_dirlist->mapToGlobal(pos));
}

void centralFrame::onCmdAdd()
{
    const QModelIndex item = _dirlist->currentIndex();
    if (!item.isValid())
        return;
    _playlist->add(fsm->fileInfo(item).absoluteFilePath());
}

void centralFrame::onCmdBmAdd()
{
    const QModelIndex item = _dirlist->currentIndex();
    if (!item.isValid())
        return;
    _bookmarkList->add(fsm->fileInfo(item).absoluteFilePath());
}

void centralFrame::changeSubtune(dir_t dir)
{
    if (_input->subtunes() <= 1)
        return;

    unsigned int i = _input->subtune();
    switch (dir)
    {
    case ID_PREV:
        i--;
        break;
    case ID_NEXT:
        i++;
        break;
    }

    if ((i < 1) || (i > _input->subtunes()))
        return;

    _audio->stop();

    if (_input->subtune(i))
        setDisplay(_input);

    if (playing)
    {
        //emit play();
        onCmdPlayPauseSong();
    }
}

void centralFrame::onCmdPlEdit(bool checked)
{
    _fileTypes->setEnabled(!checked);

    if (checked)
    {
        _playlist->clear();
        _playlist->setAcceptDrops(true);
         // FIXME this sucks
        QStringList ext = _input->ext();
        QStringList result;
        foreach (QString str, ext)
        {
            result << str.prepend("*.");
        }
        fsm->setNameFilters(result);
    }
    else
    {
        _playlist->setAcceptDrops(false);
        fsm->setNameFilters(TFACTORY->plExt());
        setProperty("AutoBackend", QVariant(false));
        onDirSelected(_dirlist->currentIndex());
    }
}

void centralFrame::onCmdPlSave()
{
    const int n = _playlist->count();
    if (!n)
        return;

    QString filter = TFACTORY->plExt().join(" ");
    QString filename = QFileDialog::getSaveFileName(this, tr("Save playlist"), "", filter);
    if (filename.isNull())
        return;

    if (!_playlist->save(filename))
        QMessageBox::critical(this, tr("Error"), tr("Error saving playlist"));
}

void centralFrame::updateSongs()
{
    if (playing && playDir.compare(fsm->fileName(_dirlist->currentIndex())))
        return;

    const int tunes = _playlist->count();
    const int tune = 1 + _playlist->currentRow();

    QString text(QString("%1/%2").arg(tune).arg(tunes));
    emit songUpdated(text);
}

void centralFrame::setDir(const QModelIndex& index)
{
    qDebug("centralFrame::setDir");
    _dirlist->setCurrentIndex(index);
    // FIXME ugly hack
    QTimer::singleShot(200, this, SLOT(scroll()));
}

void centralFrame::scroll()
{
    _dirlist->scrollTo(_dirlist->currentIndex());
}
