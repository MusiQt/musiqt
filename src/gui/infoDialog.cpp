/*
 *  Copyright (C) 2006-2025 Leandro Nini
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
#include "metaData.h"

#include <QApplication>
#include <QBuffer>
#include <QButtonGroup>
#include <QDir>
#include <QDebug>
#include <QFont>
#include <QImage>
#include <QImageReader>
#include <QLabel>
#include <QLineEdit>
#include <QSize>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QGridLayout>
#include <QThreadPool>
#include <QRegularExpression>
#include <QStackedWidget>
#include <QWidget>

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
    setObjectName("Info Dialog");

    QVBoxLayout *main = new QVBoxLayout(this);
    QHBoxLayout *container = new QHBoxLayout();
    main->addLayout(container);

    m_imgFrame = new QLabel(this);
    m_imgFrame->setFixedSize(IMAGESIZE, IMAGESIZE);
    m_imgFrame->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    m_imgFrame->setPixmap(QPixmap(":/resources/cover_placeholder.png"));
    container->addWidget(m_imgFrame);

    m_matrix = new QWidget(this);
    QGridLayout* gLayout = new QGridLayout(m_matrix);
    container->addWidget(m_matrix);

    /******************************************/

    m_extra = new QWidget(this);
    main->addWidget(m_extra);

    QVBoxLayout *extra = new QVBoxLayout(m_extra);
    extra->setContentsMargins(0,0,0,0);

    QStackedWidget *switcher = new QStackedWidget(this);
    extra->addWidget(switcher);

    QHBoxLayout *buttons = new QHBoxLayout();
    buttons->setContentsMargins(0,0,0,0);
    buttons->setSpacing(0);
    extra->addLayout(buttons);

    QButtonGroup *buttonGroup = new QButtonGroup(this);
    connect(buttonGroup, &QButtonGroup::idClicked, switcher, &QStackedWidget::setCurrentIndex);

    m_commentButton = new QPushButton(this);
    m_commentButton->setText(tr("Comment"));
    m_commentButton->setCheckable(true);
    m_commentButton->setAutoExclusive(true);
    buttonGroup->addButton(m_commentButton, 0);
    buttons->addWidget(m_commentButton);

    m_lyricsButton = new QPushButton(this);
    m_lyricsButton->setText(tr("Lyrics"));
    m_lyricsButton->setCheckable(true);
    m_lyricsButton->setAutoExclusive(true);
    buttonGroup->addButton(m_lyricsButton, 1);
    buttons->addWidget(m_lyricsButton);

    QFont font("monospace");
    font.setStyleHint(QFont::TypeWriter);

    m_comment = new QPlainTextEdit(this);
    m_comment->setReadOnly(true);
    m_comment->setFont(font);
    //m_comment->setLineWrapMode(QPlainTextEdit::WidgetWidth);
    //m_comment->setLineWrapColumnOrWidth(80);
    switcher->addWidget(m_comment);

    m_lyrics = new QPlainTextEdit(this);
    m_lyrics->setReadOnly(true);
    m_lyrics->setFont(font);
    switcher->addWidget(m_lyrics);

    {
        QWidget *matrix = new QWidget(this);
        QGridLayout* gLayout2 = new QGridLayout(matrix);
        main->addWidget(matrix);

        QLabel *lbl = new QLabel(QString("<i>file</i>:"), m_matrix);
        gLayout2->addWidget(lbl, 0, 0);
        QPalette palette = lbl->palette();
        QColor color = palette.color(lbl->foregroundRole());
        color.setAlpha(128);
        palette.setColor(lbl->foregroundRole(), color);
        lbl->setPalette(palette);

        m_location = new QLineEdit(m_matrix);
        m_location->setReadOnly(true);
        gLayout2->addWidget(m_location, 0, 1);

        lbl = new QLabel(QString("<i>backend</i>:"), m_matrix);
        gLayout2->addWidget(lbl, 1, 0);
        palette = lbl->palette();
        color = palette.color(lbl->foregroundRole());
        color.setAlpha(128);
        palette.setColor(lbl->foregroundRole(), color);
        lbl->setPalette(palette);

        m_backend = new QLabel(m_matrix);
        gLayout2->addWidget(m_backend, 1, 1);
    }

    {
        QFrame* line = new QFrame(this);
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        main->addWidget(line);
    }

    QPushButton* b = new QPushButton(GET_ICON(icon_dialogcancel), tr("&Close"), this);
    main->addWidget(b);
    connect(b, &QPushButton::clicked, this, &infoDialog::close);

    layout()->setSizeConstraint(QLayout::SetFixedSize);
}

infoDialog::~infoDialog() = default;

void infoDialog::setInfo(const metaData* mtd)
{
    m_comment->clear();
    m_lyrics->clear();

    // remove old widgets
    while (QWidget* w = m_matrix->findChild<QWidget*>())
        delete w;

    QGridLayout* gLayout = (QGridLayout*)m_matrix->layout();

    QString location = mtd->getInfo(metaData::URL);
    if (location.isEmpty())
    {
        gLayout->addWidget(new QLabel(tr("No song loaded"), m_matrix));
        setDefaultImage();
        m_extra->hide();
        return;
    }

    m_location->setText(location);
    m_backend->setText(mtd->getBackendName());

    m_commentButton->setEnabled(false);
    m_lyricsButton->setEnabled(false);

    bool showExtraBox = false;

    int j = -1;
    while ((j = mtd->moreInfo(j)) >= 0)
    {
        QString key = mtd->getKey(j);
        if (!key.compare(mtd->getKey(metaData::URL)))
            continue;

        QString info = mtd->getInfo(j);
        if (info.isEmpty())
            continue;

        // Count rows and columns
        unsigned int rows = 1;
        unsigned int cols = 0;
        unsigned int cnt = 0;
        const int len = info.length();
        for (int pos=0; pos<len; pos++)
        {
            if (info[pos] == QChar::CarriageReturn) // MacOS
            {
                if (info[pos+1] == QChar::LineFeed) // Windows
                    pos++;
                rows++;
                cnt = 0;
            }
            else if (info[pos] == QChar::LineFeed)  // UNIX
            {
                rows++;
                cnt = 0;
            }
            else
            {
                cnt++;
                if (cnt > cols)
                    cols = cnt;
            }
        }

        qDebug() << "Rows:" << rows;
        qDebug() << "Cols:" << cols;

        const bool isComment = !key.compare(mtd->getKey(metaData::COMMENT));

        if (isComment && ((rows>1) || (cols>80)))
        {
            // Comment with more than one row, or very long
            m_comment->setPlainText(info);
            m_commentButton->setEnabled(true);
            m_commentButton->click();
            showExtraBox = true;
        }
        else if (!key.compare(mtd->getKey(metaData::AS_TEXT)))
        {
            // lyrics
            m_lyrics->setPlainText(info);
            m_lyricsButton->setEnabled(true);
            if (!showExtraBox)
                m_lyricsButton->click();
            showExtraBox = true;
        }
        else
        {
            // short comments or other
            QLabel *lbl = new QLabel(QString("<i>%1</i>:").arg(key), m_matrix);
            gLayout->addWidget(lbl, j, 0);
            QPalette palette = lbl->palette();
            QColor color = palette.color(lbl->foregroundRole());
            color.setAlpha(128);
            palette.setColor(lbl->foregroundRole(), color);
            lbl->setPalette(palette);

            QLabel *textLabel = new QLabel(m_matrix);

            // trim long items
            if (cols > 80)
            {
                textLabel->setToolTip(info);
                info = info.left(75).append("[...]");
            }

            // check for URLs in comments
            if (isComment)
            {
                QRegularExpression url(R"((\b(?:https?://|www\.)\S+\b))");
                info.replace(url, R"(<a href="\1">\1</a>)");
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
        QBuffer buffer(img);
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

            qDebug() << "Trying to load album art:" << file;
            imageLoader* loader = new imageLoader(file);
            loader->setAutoDelete(true);
            connect(loader, &imageLoader::loaded,
                [this](const QImage* img) {
                    if (img != nullptr)
                    {
                        m_imgFrame->setPixmap(QPixmap::fromImage(*img));
                        delete img;
                    }
                    else
                    {
                        qDebug() << "Error loading album art";
                        setDefaultImage();
                    }

                    QApplication::restoreOverrideCursor();
                }
            );
            QThreadPool::globalInstance()->start(loader);
            QApplication::setOverrideCursor(Qt::WaitCursor); // setCursor(Qt::WaitCursor); ???
        }
        else
        {
            qDebug() << "No album art found";
            setDefaultImage();
        }
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

void infoDialog::setDefaultImage() const
{
    m_imgFrame->setPixmap(QPixmap(":/resources/cover_placeholder.png"));
}
