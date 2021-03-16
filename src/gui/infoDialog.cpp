/*
 *  Copyright (C) 2006-2021 Leandro Nini
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

#include "infoDialog.h"
#include "settings.h"
#include "iconFactory.h"
#include "utils.h"
#include "input/metaData.h"

#include <QApplication>
#include <QBuffer>
#include <QButtonGroup>
#include <QDir>
#include <QDebug>
#include <QFont>
#include <QImageReader>
#include <QSize>
#include <QPushButton>
#include <QGridLayout>
#include <QThreadPool>
#include <QRegExp>
#include <QStackedWidget>
#include <QPushButton>

constexpr int IMAGESIZE = 150;

void imageLoader::run()
{
    QImageReader reader(m_name);
    reader.setScaledSize(QSize(IMAGESIZE, IMAGESIZE));
    QImage* image = new QImage();
    bool res = reader.read(image);
    if (!res)
        utils::delPtr(image);

    emit loaded(image);
}

/*****************************************************************/

infoDialog::infoDialog(QWidget* w) :
    QDialog(w),
    m_imgFrame(nullptr)
{
    QVBoxLayout *main = new QVBoxLayout();
    setLayout(main);
    QHBoxLayout *container = new QHBoxLayout();
    main->addLayout(container);

    m_imgFrame = new QLabel(this);
    m_imgFrame->setFixedSize(IMAGESIZE, IMAGESIZE);
    m_imgFrame->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    m_imgFrame->setPixmap(QPixmap(":/resources/cover_placeholder.png"));
    container->addWidget(m_imgFrame);

    m_matrix = new QWidget(this);
    QGridLayout* gLayout = new QGridLayout();
    m_matrix->setLayout(gLayout);
    container->addWidget(m_matrix);

    /******************************************/

    m_extra = new QWidget();
    main->addWidget(m_extra);

    QVBoxLayout *extra = new QVBoxLayout();
    extra->setContentsMargins(0,0,0,0);
    m_extra->setLayout(extra);

    QStackedWidget *switcher = new QStackedWidget(this);
    extra->addWidget(switcher);

    QHBoxLayout *buttons = new QHBoxLayout();
    buttons->setContentsMargins(0,0,0,0);
    buttons->setSpacing(0);
    extra->addLayout(buttons);

    QButtonGroup *buttonGroup = new QButtonGroup(this);
    connect(buttonGroup, &QButtonGroup::idClicked, switcher, &QStackedWidget::setCurrentIndex);

    QPushButton* button = new QPushButton(this);
    button->setText(tr("Comment"));
    button->setCheckable(true);
    button->setAutoExclusive(true);
    button->setChecked(true);
    buttonGroup->addButton(button, 0);
    buttons->addWidget(button);

    button = new QPushButton(this);
    button->setText(tr("Lyrics"));
    button->setCheckable(true);
    button->setAutoExclusive(true);
    //button->setEnabled(false);
    buttonGroup->addButton(button, 1);
    buttons->addWidget(button);

    QFont font("monospace");
    font.setStyleHint(QFont::TypeWriter);

    m_comment = new QPlainTextEdit();
    m_comment->setReadOnly(true);
    m_comment->setFont(font);
    //m_comment->setLineWrapMode(QPlainTextEdit::WidgetWidth);
    //m_comment->setLineWrapColumnOrWidth(80);
    switcher->addWidget(m_comment);

    m_lyrics = new QPlainTextEdit();
    m_lyrics->setReadOnly(true);
    m_lyrics->setFont(font);
    switcher->addWidget(m_lyrics);

    {
        QFrame* line = new QFrame();
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        main->addWidget(line);
    }

    QPushButton* b = new QPushButton(GET_ICON(icon_dialogcancel), tr("&Close"), this);
    main->addWidget(b);
    connect(b, &QPushButton::clicked, this, &infoDialog::close);

    layout()->setSizeConstraint(QLayout::SetFixedSize);
}

infoDialog::~infoDialog() {}

void infoDialog::setInfo(const metaData* mtd)
{
    m_comment->clear();
    m_lyrics->clear();

    // remove old widgets
    while (QWidget* w = m_matrix->findChild<QWidget*>())
        delete w;

    QGridLayout* gLayout = (QGridLayout*)m_matrix->layout();

    QString location = mtd->getInfo(metaData::LOCATION);
    if (location.isEmpty())
    {
        gLayout->addWidget(new QLabel(tr("No song loaded"), m_matrix));
        m_extra->hide();
        return;
    }

    bool showExtraBox = false;
    int j = -1;
    while ((j = mtd->moreInfo(j)) >= 0)
    {
        QString key = mtd->getKey(j);
        if (!key.compare(mtd->getKey(metaData::LOCATION)))
            continue;

        QString info = mtd->getInfo(j);
        if (info.isEmpty())
            continue;

        unsigned int rows = 1;
        unsigned int cols = 0;
        unsigned int cnt = 0;
        const int len = info.length();
        for (int pos=0; pos<len; pos++)
        {
            // Convert Mac and Windows EOLs to Unix
            if (info[pos] == '\r')
                info.replace(pos, (info[pos+1]=='\n') ? ' ' : '\n');
            // Count rows and columns
            if (info[pos] == '\n')
            {
                rows++;
                if (cnt > cols)
                    cols = cnt;
                cnt = 0;
            } else
                cnt++;
        }
        if (cnt > cols)
            cols = cnt;

        qDebug() << "Rows: " << rows;
        qDebug() << "Cols: " << cols;

        const bool isComment = !key.compare(mtd->getKey(metaData::COMMENT));

        if (isComment && ((rows>1) || (cols>80)))
        {
            m_comment->setPlainText(info);
            showExtraBox = true;
        }
        else if (!key.compare(mtd->getKey(metaData::LYRICS)))
        {
            m_lyrics->setPlainText(info);
            showExtraBox = true;
        }
        else
        {
            QLabel *lbl = new QLabel(QString("<i>%1</i>:").arg(key), m_matrix);
            gLayout->addWidget(lbl, j, 0);
            QPalette palette = lbl->palette();
            QColor color = palette.color(lbl->foregroundRole());
            color.setAlpha(128);
            palette.setColor(lbl->foregroundRole(), color);
            lbl->setPalette(palette);

            QLabel *textLabel = new QLabel(m_matrix);

            if (cols > 80)
            {
                textLabel->setToolTip(info);
                info = info.left(75).append("[...]");
            }

            if (isComment)
            {
                QRegExp url("(\\b(?:https?://|www\\.)\\S+\\b)");
                info.replace(url, "<a href=\"\\1\">\\1</a>");
                textLabel->setOpenExternalLinks(true);
            }
            textLabel->setText(info);
            gLayout->addWidget(textLabel, j, 1);
        }
    }

    // Display album art
    QImage newimg;
    QByteArray *img = mtd->getImage();
    if (img != nullptr)
    {
        QByteArray ba(img->data(), img->size());
        QBuffer buffer(&ba);
        if (buffer.open(QIODevice::ReadOnly))
        {
            QImageReader reader(&buffer);
            reader.setScaledSize(QSize(IMAGESIZE, IMAGESIZE));
            newimg = reader.read();
            buffer.close();
        }
    }

    if (!newimg.isNull())
    {
        qDebug() << "Using embedded cover art";
        m_imgFrame->setPixmap(QPixmap::fromImage(newimg));
    }
    else
    {
        QDir dir = QFileInfo(location).dir();
        QString folder = dir.path();
        QStringList filters;
        filters << "cover.*" << "front.*" << "folder.*";
        QStringList files = dir.entryList(filters, QDir::Files);

        if (files.size() > 0)
        {
            QString file = QFileInfo(folder, files[0]).canonicalFilePath();

            qDebug() << "Trying to load album art: " << file;
            imageLoader* loader = new imageLoader(file);
            loader->setAutoDelete(true);
            connect(loader, &imageLoader::loaded,
                [this](const QImage* img) {
                    if (img != nullptr)
                    {
                        m_imgFrame->setPixmap(QPixmap::fromImage(*img));
                        delete img;
                    }

                    QApplication::restoreOverrideCursor();
                }
            );
            QThreadPool::globalInstance()->start(loader);
            QApplication::setOverrideCursor(Qt::WaitCursor); // setCursor(Qt::WaitCursor); ???
        }

        m_imgFrame->setPixmap(QPixmap(":/resources/cover_placeholder.png"));
    }

    if (gLayout->count() == 0)
        gLayout->addWidget(new QLabel(tr("No info"), m_matrix));

    if (showExtraBox)
    {
        m_extra->show();
    }
    else
    {
        m_extra->hide();
    }

    gLayout->update();
    adjustSize();
}
