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

#include "centralFrame.h"

#include "iconFactory.h"
#include "bookmark.h"
#include "playlist.h"
#include "playlistModel.h"
#include "proxymodel.h"
#include "input/inputFactory.h"
#include "input/input.h"
#include "input/metaData.h"
#include "settings.h"
#include "trackListFactory.h"
#include "xdg.h"

#include <QApplication>
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QFileInfo>
#include <QFileSystemModel>
#include <QItemSelectionModel>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QSlider>
#include <QStackedWidget>
#include <QTimer>
#include <QTreeView>
#include <QHeaderView>
#include <QWidgetAction>
#include <QDebug>


void loadThread::run()
{
    if (!iBackend->open(fileName))
    {
        utils::delPtr(iBackend);
    }
    emit loaded(iBackend);
}

/*****************************************************************/

centralFrame::centralFrame(QWidget *parent) :
    QWidget(parent),
    _input(IFACTORY->get()),
    _preload(nullptr),
    _audio(new audio),
    playing(false),
    preloaded(QString()),
    playDir(QString())
{
    //connect(_audio, &audio::outputError, this, &onCmdStopSong);
    connect(_audio, &audio::updateTime,  this, &centralFrame::onUpdateTime);
    connect(_audio, &audio::songEnded,   this, &centralFrame::songEnded);
    connect(_audio, &audio::preloadSong, this, &centralFrame::preloadSong);

    // dir view
    fsm = new QFileSystemModel(this);
    fsm->setFilter(QDir::AllDirs|QDir::Drives|QDir::NoDotAndDotDot|QDir::Files);
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
    _dirlist->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    _dirlist->header()->setStretchLastSection(false);
    QItemSelectionModel* selectionModel = _dirlist->selectionModel();

    connect(selectionModel, &QItemSelectionModel::currentChanged, this, &centralFrame::onDirSelected);
    connect(_dirlist, &QTreeView::customContextMenuRequested, this, &centralFrame::onRgtClkDirList);

    // _playlist
    _playlist = new playlist(this);
    _playlist->setAlternatingRowColors(true);
    _playlist->setUniformItemSizes(true);
    _playlist->setDragDropMode(QAbstractItemView::DropOnly);

    _playlistModel = new playlistModel(this);

    _proxyModel = new proxymodel(this);
    _proxyModel->setSourceModel(_playlistModel);
    _proxyModel->setDynamicSortFilter(true);
    _proxyModel->sort(proxymodel::sortMode::Ascending);
    _playlist->setModel(_proxyModel);

    _proxyModel->setFilterRegExp(QRegExp(getFilter(), Qt::CaseInsensitive));
    _proxyModel->setFilterRole(Qt::UserRole+1);

    selectionModel = _playlist->selectionModel();
    connect(selectionModel, &QItemSelectionModel::currentRowChanged, this, &centralFrame::onCmdSongSelected);
    connect(_playlist, &playlist::doubleClicked, this, &centralFrame::onCmdPlayPauseSong);
    connect(_playlist, &playlist::changed, this, &updateSongs);

    _playlist->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(_playlist, &playlist::customContextMenuRequested, this, &onRgtClkPlayList);

    _bookmarkList = new bookmark(this);
    _bookmarkList->setAlternatingRowColors(true);
    connect(_bookmarkList, &bookmark::currentTextChanged, this, &gotoDir);

    // left view
    QStackedWidget *stackedWidget = new QStackedWidget(this);
    stackedWidget->addWidget(_playlist);
    stackedWidget->addWidget(_bookmarkList);
    QButtonGroup *buttonGroup = new QButtonGroup(this);
    connect(buttonGroup, &QButtonGroup::idClicked, stackedWidget, &QStackedWidget::setCurrentIndex);

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
        _editMode = new QPushButton(this);
        _editMode->setIcon(GET_ICON(icon_editlist));
        _editMode->setToolTip(tr("Edit"));;
        _editMode->setStatusTip(tr("Edit playlist"));
        _editMode->setCheckable(true);
        buttons->addWidget(_editMode);
        connect(_editMode, &QPushButton::clicked, this, &onCmdPlEdit);
        QPushButton *b1 = new QPushButton(this);
        b1->setIcon(GET_ICON(icon_documentsave));
        b1->setToolTip(tr("Save"));
        b1->setStatusTip(tr("Save playlist"));
        connect(b1, &QPushButton::clicked, this, &onCmdPlSave);
        buttons->addWidget(b1);
        b1 = new QPushButton(this);
        b1->setIcon(GET_ICON(icon_currentplaylist));
        b1->setToolTip(tr("Current playlist"));
        b1->setStatusTip(tr("Return to current playlist"));
        connect(b1, &QPushButton::clicked, this, &onCmdCurrentDir);
        buttons->addWidget(b1);
        m_home = new QPushButton(this);
        m_home->setIcon(GET_ICON(icon_gohome));
        m_home->setToolTip(tr("Home"));
        m_home->setStatusTip(tr("Home"));
        m_home->setMenu(new QMenu());

        createHomeMenu();

        buttons->addWidget(m_home);
        buttons->setSpacing(0);
    }

    // Left view - file list/bookmarks
    QWidget *w2 = new QWidget(this);
    QVBoxLayout *vbox = new QVBoxLayout(w2);
    vbox->setContentsMargins(0,0,0,0);
    vbox->addWidget(w);
    vbox->addWidget(stackedWidget);
    vbox->addWidget(wButtons);
    vbox->setSpacing(0);

    QWidget *cFrame = new QWidget(this);
    QHBoxLayout *hbox = new QHBoxLayout(cFrame);
    hbox->setContentsMargins(0,0,0,0);
    hbox->addWidget(w2);
    hbox->addWidget(_dirlist);

    QVBoxLayout *main = new QVBoxLayout(this);
    main->setContentsMargins(0,0,0,0);
    m_slider = new QSlider(Qt::Horizontal);
    m_slider->setMinimum(0);
    m_slider->setMaximum(100);
    m_slider->setTickInterval(10);
    m_slider->setTickPosition(QSlider::TicksBelow);
    m_slider->setTracking(false);
    m_slider->setDisabled(true);
    connect(m_slider, &QSlider::actionTriggered, this, &centralFrame::onSeek);
    connect(this, &updateSlider, m_slider, &QSlider::setValue);
    main->addWidget(m_slider);
    main->addWidget(cFrame);
}

centralFrame::~centralFrame()
{
    _bookmarkList->save();

    QSettings settings;
    QString file = _input->songLoaded();
    settings.setValue("General Settings/file", file);

    delete _audio;
    delete _input;
}

void centralFrame::createHomeMenu()
{
    QMenu *menu = m_home->menu();
    menu->clear();

    QActionGroup *homeGroup = new QActionGroup(menu);

    QString musicDir = xdg::getMusicDir();
    if (!musicDir.isEmpty())
    {
        QAction *action = menu->addAction(tr("System music location"));
        action->setData(musicDir);
        action->setStatusTip(musicDir);
        homeGroup->addAction(action);
    }

    for (int i=0; i<IFACTORY->num(); i++)
    {
        inputConfig* ic = IFACTORY->getConfig(i);

        QString musicDir = ic->getMusicDir();
        if (!musicDir.isEmpty())
        {
            QString name = IFACTORY->name(i);
            QAction *action = menu->addAction(ic->icon(), name.append(tr(" music location")));
            action->setData(musicDir);
            action->setStatusTip(musicDir);
            homeGroup->addAction(action);
        }

        delete ic;
    }
    connect(homeGroup, &QActionGroup::triggered, this, &onHome);
}

void centralFrame::changeState()
{
    emit stateChanged(_audio->state());
    switch (_audio->state())
    {
    case state_t::STOP:
        playing = false;
        m_slider->setDisabled(true);
        break;
    case state_t::PLAY:
        playing = true;
        m_slider->setDisabled(!_input->seekable());
        break;
    case state_t::PAUSE:
        m_slider->setDisabled(true);
        break;
    }
}

void centralFrame::onDirSelected(const QModelIndex& idx)
{
    _bookmarkList->setCurrentRow(-1);

    if (_editMode->isChecked())
        return;

    const QString curItem = fsm->fileName(_dirlist->currentIndex());
    if (curItem.isEmpty())
        return;

    qDebug() << "Set dir " << curItem;

    const QString mPath = fsm->fileInfo(idx).absoluteFilePath();
    qDebug() << "Full path: " << mPath;

    _playlistModel->load(mPath);

    if (_proxyModel->rowCount() == 0)
    {
        return;
    }

    const QString fileName = _dirlist->property("UserData").toString();
    if (!fileName.isEmpty())
    {
        qDebug() << "selecting file " << fileName;
        QModelIndexList items = _proxyModel->match(_proxyModel->index(0, 0), Qt::DisplayRole,
                QVariant::fromValue(fileName), 1, Qt::MatchExactly|Qt::MatchCaseSensitive);
        if (!items.empty())
        {
            _playlist->setCurrentIndex(items.at(0));
            _dirlist->setProperty("UserData", QVariant(QString()));
        }
    }
    else
    {
        QString songLoaded = _input->songLoaded();
        if (songLoaded.isEmpty())
        {
            _playlist->setCurrentIndex(_proxyModel->index(0, 0));
        }
        else
        {
            QFileInfo fileInfo(songLoaded);
            QModelIndexList items = _proxyModel->match(_proxyModel->index(0, 0), Qt::DisplayRole,
                    QVariant::fromValue(fileInfo.completeBaseName()), 1, Qt::MatchExactly|Qt::MatchCaseSensitive);
            if (!items.empty())
            {
                _playlist->setCurrentIndex(items.at(0));
                _playlist->scrollTo(items.at(0));
            }
            else if (!playing)
                _playlist->setCurrentIndex(_proxyModel->index(0, 0));
        }
    }
}

void centralFrame::onHome(QAction* action)
{
    QString musicDir = action->data().toString();
    qDebug() << "go to music dir: " << musicDir;
    QModelIndex idx = fsm->index(musicDir);
    if (idx.isValid())
        setDir(idx);
    else
        QMessageBox::warning(this, tr("Warning"), QString(tr("Path %1 dos not exist")).arg(musicDir));
}

void centralFrame::onCmdCurrentDir()
{
    if (_editMode->isChecked())
        return;

    qDebug() << "onCmdCurrentDir";
    QString file = _input->songLoaded();
    if (file.isEmpty())
        return;

    QFileInfo fileInfo(file);
    gotoDir(fileInfo.absolutePath());
    QModelIndexList items = _proxyModel->match(_proxyModel->index(0, 0),
        Qt::DisplayRole, QVariant::fromValue(fileInfo.completeBaseName()),
        -1, Qt::MatchExactly|Qt::MatchCaseSensitive);

    _playlist->setCurrentIndex(items.at(0));
}

void centralFrame::gotoDir(const QString &dir)
{
    qDebug() << "gotoDir: " << dir;
    if (!dir.isEmpty())
        setDir(fsm->index(dir));
}
 
void centralFrame::setFile(const QString& file, const bool play)
{
    qDebug() << "setFile " << file;

    QModelIndex curItem = _playlist->currentIndex();

    QFileInfo fileInfo(file);
    // Check if requested file/directory exists
    if (!fileInfo.exists())
    {
        // ???
        //if (curItem.isValid())
        //    onHome();
        return;
    }

    QModelIndexList items = _playlistModel->match(_playlistModel->index(0, 0), Qt::DisplayRole, QVariant::fromValue(file), -1, Qt::MatchExactly|Qt::MatchCaseSensitive);

    const bool selected = items.empty() ? false : _playlist->selectionModel()->isSelected(items.at(0));

    // Check if requested song is already playing
    if (playing && selected)
        return;

    QModelIndex val;
    if (!items.empty())
        val = items.at(0);

    QString currentDir = fsm->fileName(_dirlist->currentIndex());
    const bool dirSelected = !currentDir.compare(fileInfo.dir().absolutePath());

    if (fileInfo.isDir() || (TFACTORY->plExt().indexOf(fileInfo.suffix().toLower()) >= 0))
    {
        if (dirSelected)
        {
            if (play && !playing)
            {
                onCmdPlayPauseSong();
            }
            return;
        }
        else
        {
            _dirlist->setProperty("UserData", QVariant(QString()));
            setDir(fsm->index(file));
            playMode = true; // TODO sync GUI
        }
        goto done;
    }

    {
        const QString fName = fileInfo.fileName();

        if (!fName.isEmpty())
        {
            QRegExp regexp = _proxyModel->filterRegExp();
            if (!regexp.exactMatch(fName))
            {
                // file is not supported, ignore message
                return;
            }
        }
    }

    if (dirSelected)
    {
        if (val.isValid())
        {
            if (val == curItem)
            {
                onCmdPlayPauseSong();
            }
            else
                _playlist->setCurrentIndex(val);
        }
    }
    else
    {
        _dirlist->setProperty("UserData", QVariant(fileInfo.completeBaseName()));
        setDir(fsm->index(fileInfo.dir().absolutePath()));
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
    else
    {
        // if loaded song is not the selected one don't play
        QString songLoaded = _input->songLoaded();
        const QString song = _proxyModel->data(_playlist->currentIndex(), Qt::UserRole).toString();
        qDebug() << "Song: " << song;
        if (!song.isEmpty() && song.compare(songLoaded))
        {
            return;
        }

        if (_audio->play(_input))
        {
            QFileInfo fileInfo(songLoaded);
            gotoDir(fileInfo.absolutePath());
            playDir = fsm->fileName(_dirlist->currentIndex());
        }
    }

    changeState();
}

void centralFrame::onCmdStopSong()
{
    qDebug() << "centralFrame::onCmdStopSong";
    if (_audio->stop())
    {
        emit updateTime(0);
        emit updateSlider(0);
        changeState();
        QModelIndex curr = _dirlist->currentIndex();
        if (playDir.compare(fsm->fileName(curr)))
        {
            onDirSelected(curr);
        }
        playDir = QString();
    }
}

void centralFrame::onCmdPrevSong()
{
    onCmdChangeSong(dir_t::ID_PREV);
}

void centralFrame::onCmdNextSong()
{
    onCmdChangeSong(dir_t::ID_NEXT);
}

void centralFrame::onCmdChangeSong(dir_t dir)
{
    qDebug() << "playDir " << playDir;
    if (playing && playDir.compare(fsm->fileName(_dirlist->currentIndex())))
        return;

    int row = _playlist->currentIndex().row();

    switch (dir)
    {
    case dir_t::ID_PREV:
            row--;
            qDebug("Previous");
            break;
    case dir_t::ID_NEXT:
            row++;
            qDebug("Next");
            break;
    }

    QModelIndex index = _proxyModel->index(row, 0);

    if (index.isValid())
    {
        if (!preloaded.isEmpty())
        {
            _preload->close();
            preloaded = QString();
        }
        _playlist->setCurrentIndex(index);
    }
}

void centralFrame::load(const QString& filename)
{
    qDebug() << "Loading " << filename;

    input *ib =  IFACTORY->get(filename);
    if (ib == nullptr)
        return;

    loadThread* loader = new loadThread(ib, filename);
    connect(loader, &loadThread::loaded, this, &onCmdSongLoaded);
    connect(loader, &loadThread::finished, loader, &loadThread::deleteLater);

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
            preloaded = QString();
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
            onCmdPlayPauseSong();
        }
        qDebug() << "Song loaded";
    }
    else
    {
        playDir = QString();
        emit setInfo(nullptr);
        changeState();
        qWarning() << "Error loading song";
    }

    QApplication::restoreOverrideCursor();
}

void centralFrame::onCmdSongSelected(const QModelIndex& currentRow)
{
    if (_editMode->isChecked())
        return;

    qDebug() << "onCmdSongSelected " << currentRow.row();
    if (!currentRow.isValid())
        return;

    QString songLoaded = _input->songLoaded();
    const QString song = _proxyModel->data(currentRow, Qt::UserRole).toString();
    if (!songLoaded.isEmpty() && !song.compare(songLoaded))
        return;

    _playlist->scrollTo(currentRow);

    if (!_playlist->isVisible())
        updateSongs();

    if (!preloaded.isEmpty())
    {
        if (preloaded.compare(song))
        {
            preloaded = QString();
            utils::delPtr(_preload);
        }
        else
        {
            delete _input;
            _input = _preload;
            _preload = nullptr;
            preloaded = QString();
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
}

void centralFrame::onUpdateTime()
{
    // Playing may have stopped while message was in queue
    if (_audio->state() != state_t::STOP)
    {
        emit updateTime(_audio->seconds());

        if (!m_slider->isSliderDown())
            emit updateSlider((100*_audio->seconds())/_input->time());
    }
}

void centralFrame::onSeek()
{
    int pos = m_slider->sliderPosition();
    qDebug() << "onSeek: " << pos;
    _audio->pause();
    _input->seek(pos);
    _audio->seek(pos); // FIXME this sucks
    _audio->pause();
}

void centralFrame::preloadSong()
{
    qDebug("centralFrame::preloadSong");
    if (playMode && _input->gapless() && !playDir.compare(fsm->fileName(_dirlist->currentIndex())))
    {
        const int nextSong = _playlist->currentIndex().row()+1;
        QModelIndex index = _proxyModel->index(nextSong, 0);
        if (index.isValid())
        {
            preloaded = _proxyModel->data(index, Qt::UserRole).toString();
            load(preloaded);
        }
    }
}

void centralFrame::songEnded()
{
    qDebug("centralFrame::songEnded");
    if (SETTINGS->subtunes() && (_input->subtune()<_input->subtunes()))
    {
        changeSubtune(dir_t::ID_NEXT);
        return;
    }

    if (playMode)
    {
        QModelIndex index;
        if (!playDir.compare(fsm->fileName(_dirlist->currentIndex())))
        {
            int nextSong = _playlist->currentIndex().row() + 1;
            index = _proxyModel->index(nextSong, 0);
        }

        if (index.isValid())
        {
            _playlist->setCurrentIndex(index);
            return;
        }
    }

    onCmdStopSong();
}

void centralFrame::onSettingsChanged()
{
    createHomeMenu();

    QString songLoaded = _input->songLoaded();
    if (!songLoaded.isEmpty())
    {
        // we must reload the song
        _input->close();
        const QModelIndex curItem = _playlist->currentIndex();
        _playlist->setCurrentIndex(QModelIndex());
        _playlist->setCurrentIndex(curItem);
    }
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
        QAction *additem = pane.addAction(GET_ICON(icon_listadd), tr("Add to playlist"), this, &centralFrame::onCmdAdd);
        additem->setStatusTip(tr("Add song to the playlist"));
    }
    else
    {
        QAction *additem = pane.addAction(GET_ICON(icon_bookmark), tr("Add to bookmarks"), this, &centralFrame::onCmdBmAdd);
        additem->setStatusTip(tr("Add directory to the bookmarks"));
    }

    //setUserData((void*)item);

    pane.exec(_dirlist->mapToGlobal(pos));
}

void centralFrame::onRgtClkPlayList(const QPoint& pos)
{
    qDebug() << "onRgtClkPlayList";
    QModelIndex item = _playlist->indexAt(pos);

    setProperty("UserData", QVariant::fromValue(item.row()));

    QMenu pane(this);
    if (item.isValid())
    {
        QWidgetAction *wa = new QWidgetAction(&pane);
        QLabel *label = new QLabel(utils::shrink(_playlistModel->data(item, Qt::UserRole).toString()));
        label->setAlignment(Qt::AlignCenter);
        wa->setDefaultWidget(label);
        pane.addAction(wa);
        pane.addSeparator();
        QAction *delitem = pane.addAction(GET_ICON(icon_listremove), tr("Remove item"), this, &centralFrame::onCmdDel);
        delitem->setStatusTip(tr("Remove selected item from playlist"));
        pane.addSeparator();
    }

    QAction* asc = new QAction(tr("Sort ascending"), &pane);
    asc->setCheckable(true);
    asc->setStatusTip(tr("Sort ascending"));
    if (_proxyModel->getMode() == proxymodel::sortMode::Ascending) asc->setChecked(true);
    connect(asc, &QAction::triggered, this, &sortAsc);
    QAction* desc = new QAction(tr("Sort descending"), &pane);
    desc->setCheckable(true);
    desc->setStatusTip(tr("Sort descending"));
    if (_proxyModel->getMode() == proxymodel::sortMode::Descending) desc->setChecked(true);
    connect(desc, &QAction::triggered, this, &sortDesc);
    QAction* rnd = new QAction(tr("Shuffle"), &pane);
    rnd->setCheckable(true);
    rnd->setStatusTip(tr("Sort randomly"));
    if (_proxyModel->getMode() == proxymodel::sortMode::Random) rnd->setChecked(true);
    connect(rnd, &QAction::triggered, this, &shuffle);

    QActionGroup *radioGroup = new QActionGroup(&pane);
    radioGroup->addAction(asc);
    radioGroup->addAction(desc);
    radioGroup->addAction(rnd);

    pane.addSeparator()->setText(tr("Sorting"));
    pane.addAction(asc);
    pane.addAction(desc);
    pane.addAction(rnd);

    pane.exec(_playlist->mapToGlobal(pos));
}

void centralFrame::sortAsc()
{
    _proxyModel->sort(proxymodel::sortMode::Ascending);
}

void centralFrame::sortDesc()
{
    _proxyModel->sort(proxymodel::sortMode::Descending);
}

void centralFrame::shuffle()
{
    _proxyModel->sort(proxymodel::sortMode::Random);
}

void centralFrame::onCmdAdd()
{
    const QModelIndex item = _dirlist->currentIndex();
    if (!item.isValid())
        return;
    _playlistModel->append(fsm->fileInfo(item).absoluteFilePath());
}

void centralFrame::onCmdDel()
{
    int row = property("UserData").toInt();
    qDebug() << "onCmdDel " << row;
    _playlistModel->removeRow(row);
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
    case dir_t::ID_PREV:
        i--;
        break;
    case dir_t::ID_NEXT:
        i++;
        break;
    }

    if ((i < 1) || (i > _input->subtunes()))
        return;

    _audio->stop();

    if (_input->subtune(i))
        emit setDisplay(_input);

    if (playing)
    {
        onCmdPlayPauseSong();
    }
}

void centralFrame::onCmdPlEdit(bool checked)
{
    _playlist->setAcceptDrops(checked);
    _playlist->setDropIndicatorShown(checked);

    if (checked)
    {
        _playlistModel->clear();

        fsm->setNameFilters(getPattern());

        setProperty("SelectedDir", QVariant(fsm->filePath(_dirlist->currentIndex())));
    }
    else
    {
        fsm->setNameFilters(TFACTORY->plExt());
        _dirlist->setCurrentIndex(QModelIndex());
        QString dir = property("SelectedDir").toString();
        qDebug() << "dir " << dir;
        _dirlist->setCurrentIndex(fsm->index(dir));
    }
}

void centralFrame::onCmdPlSave()
{
    const int n = _proxyModel->rowCount();
    if (!n)
        return;

    QString filter = TFACTORY->plExt().join(" ");
    QString filename = QFileDialog::getSaveFileName(this, tr("Save playlist"), "", filter);
    if (filename.isEmpty())
        return;

    std::unique_ptr<trackList> tracklist(TFACTORY->get(filename));

    if (tracklist.get() == nullptr)
        return;

    tracks_t tracks;
    for(int r = 0; r < _proxyModel->rowCount(); ++r)
    {
        QModelIndex idx = _proxyModel->index(r, 0);
        tracks.append(_proxyModel->data(idx, Qt::UserRole+1).toString());
    }
    if (!tracklist->save(tracks))
        QMessageBox::critical(this, tr("Error"), tr("Error saving playlist"));
}

void centralFrame::updateSongs()
{
    if (playing && playDir.compare(fsm->fileName(_dirlist->currentIndex())))
        return;

    const int tunes = _proxyModel->rowCount();
    const int tune = 1 + _playlist->currentIndex().row();

    QString text(QString("%1/%2").arg(tune).arg(tunes));
    emit songUpdated(text);
}

void centralFrame::setDir(const QModelIndex& index)
{
    qDebug("centralFrame::setDir");
    QMetaObject::Connection connection = connect(
        fsm, &QFileSystemModel::directoryLoaded,
        [connection, this](const QString &path) {
            qDebug() << "dirLoaded: " << path;
            if (path.compare(fsm->fileInfo(_dirlist->currentIndex()).absolutePath()) == 0)
            {
                qDebug() << "scrollTo" << path;
                QObject::disconnect(connection);
                QTimer::singleShot(50, [this](){_dirlist->scrollTo(_dirlist->currentIndex());});
            }
        }
    );

    _dirlist->setCurrentIndex(index);
    _dirlist->scrollTo(index);
}

void centralFrame::init()
{
    qDebug() << "*** init ***";
    fsm->setRootPath(QDir::rootPath());

    _bookmarkList->load();
}

const metaData* centralFrame::getMetaData() const { return _input->getMetaData(); }

QString centralFrame::getFilter() const
{
    QString filter(IFACTORY->getExtensions().join("|"));
    filter.prepend(".*\\.(").append(")");
    qDebug() << "filter: " << filter;
    return filter;
}

QStringList centralFrame::getPattern() const
{
    QStringList result;
    for (QString str: IFACTORY->getExtensions())
    {
        result << str.prepend("*.");
    }
    qDebug() << "pattern: " << result;
    return result;
}
