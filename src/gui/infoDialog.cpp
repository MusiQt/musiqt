/*
 *  Copyright (C) 2006-2019 Leandro Nini
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
#include <QDir>
#include <QDebug>
#include <QFont>
#include <QImageReader>
#include <QSize>
#include <QPushButton>
#include <QGridLayout>
#include <QThreadPool>
#include <QRegExp>

constexpr int IMAGESIZE = 150;

void imageLoader::run()
{
    QImageReader reader(name);
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
    imgFrame(nullptr)
{
    QVBoxLayout *main = new QVBoxLayout();
    setLayout(main);
    QHBoxLayout *container = new QHBoxLayout();
    main->addLayout(container);

    imgFrame = new QLabel(this);
    imgFrame->setFixedSize(IMAGESIZE, IMAGESIZE);
    imgFrame->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    imgFrame->setPixmap(QPixmap(":/resources/cover_placeholder.png"));
    container->addWidget(imgFrame);

    matrix = new QWidget(this);
    QGridLayout* gLayout = new QGridLayout();
    matrix->setLayout(gLayout);
    container->addWidget(matrix);

    QFont font("monospace");
    font.setStyleHint(QFont::TypeWriter);
    text = new QPlainTextEdit(this);
    text->setReadOnly(true);
    text->setFont(font);
    //text->setLineWrapMode(QPlainTextEdit::WidgetWidth);
    //text->setLineWrapColumnOrWidth(80);
    main->addWidget(text);

    {
        QFrame* line = new QFrame();
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        main->addWidget(line);
    }

    QPushButton* b = new QPushButton(GET_ICON(icon_dialogcancel), tr("&Close"), this);
    main->addWidget(b);
    connect(b, SIGNAL(clicked()), this, SLOT(close()));
}

infoDialog::~infoDialog() {}

void infoDialog::setInfo(const metaData* mtd)
{
    text->clear();
    //text->setVisibleRows(0);
    //text->setVisibleColumns(0);

    // remove old widgets
    while (QWidget* w = matrix->findChild<QWidget*>())
        delete w;

    QGridLayout* gLayout = (QGridLayout*)matrix->layout();

    QString location = mtd->getInfo(metaData::LOCATION);
    if (location.isEmpty())
    {
        gLayout->addWidget(new QLabel(tr("No song loaded"), matrix));
        text->hide();
        return;
    }

    int j = -1;
    while ((j = mtd->moreInfo(j)) >= 0)
    {
        QString temp = mtd->getInfo(j);
        if (temp.isEmpty())
            continue;

        unsigned int rows = 1;
        unsigned int cols = 0;
        unsigned int cnt = 0;
        int pos = 0;
        const int len = temp.length();
        const char* s = temp.toLocal8Bit().constData();
        while (pos < len)
        {
            // Convert Mac and Windows EOLs to Unix
            if (s[pos] == '\r')
                temp.replace(pos, (s[pos+1]=='\n') ? '\0' : '\n');
            // Count rows and columns
            if (s[pos] == '\n')
            {
                rows++;
                if (cnt > cols)
                    cols = cnt;
                cnt = 0;
            } else
                cnt++;
            //pos += QString::utfBytes[(unsigned char)s[pos]];
            pos++;
        }
        if (cnt > cols)
            cols = cnt;

        qDebug() << "Rows: " << rows;
        qDebug() << "Cols: " << cols;

        if (rows>1)
        {
            text->setPlainText(temp);
            //text->setVisibleRows((rows<20)?rows:20);
            //text->setLineWrapColumnOrWidth((cols<80)?cols+2:80);
        }
        else
        {
            const QString info = mtd->getKey(j);
            if (QString::compare(info, "location")) // FIXME
            {
                QLabel *lbl = new QLabel(QString("<i>%1</i>:").arg(info), matrix);
                gLayout->addWidget(lbl, j, 0);
                QPalette palette = lbl->palette();
                QColor color = palette.color(lbl->foregroundRole());
                color.setAlpha(128);
                palette.setColor(lbl->foregroundRole(), color);
                lbl->setPalette(palette);

                QLabel *textLabel = new QLabel(matrix);
                if (!QString::compare(info, "comment"))
                {
                    QRegExp url("(\\b(?:https?://|www\\.)\\S+\\b)");
                    temp.replace(url, "<a href=\"\\1\">\\1</a>");
                    textLabel->setOpenExternalLinks(true);
                }
                textLabel->setText(temp);
                gLayout->addWidget(textLabel, j, 1);
            }
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
        imgFrame->setPixmap(QPixmap::fromImage(newimg));
    }
    else
    {
        QDir dir = QFileInfo(location).dir();
        QString folder = dir.path();
        QStringList filters;
        filters << "cover.*" << "front.*" << "folder.*";
        QStringList _files = dir.entryList(filters, QDir::Files);

        if (_files.size() > 0)
        {
            QString file = QFileInfo(folder, _files[0]).canonicalFilePath();

            qDebug() << "Trying to load album art: " << file;
            imageLoader* loader = new imageLoader(file);
            loader->setAutoDelete(true);
            connect(loader, SIGNAL(loaded(const QImage*)), this, SLOT(onImgLoaded(const QImage*)));
            QThreadPool::globalInstance()->start(loader);
            QApplication::setOverrideCursor(Qt::WaitCursor); // setCursor(Qt::WaitCursor); ???
        }

        imgFrame->setPixmap(QPixmap(":/resources/cover_placeholder.png"));
    }

    if (gLayout->count() == 0)
        gLayout->addWidget(new QLabel(tr("No info"), matrix));

    qDebug() << "Comment characters " << text->document()->characterCount();
    if (text->document()->characterCount() > 1)
    {
        text->show();
        //text->resize(text->getDefaultWidth(), text->getDefaultHeight());
    }
    else
    {
        text->hide();
    }

    gLayout->update();
    adjustSize();
}

void infoDialog::onImgLoaded(const QImage* img)
{
    if (img != nullptr)
    {
        imgFrame->setPixmap(QPixmap::fromImage(*img));
        delete img;
    }

    QApplication::restoreOverrideCursor();
}
