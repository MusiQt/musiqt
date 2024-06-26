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

#ifndef INFODIALOG_H
#define INFODIALOG_H

#include <QDialog>
#include <QRunnable>

class metaData;
class QImage;
class QLabel;
class QPlainTextEdit;
class QPushButton;
class QWidget;

class imageLoader : public QObject, public QRunnable
{
    Q_OBJECT

private:
    QString m_name;

public:
    void run() override;

public:
    imageLoader(const QString& file) :
        m_name(file)
    {}

signals:
    void loaded(const QImage* image);
};

/*****************************************************************/

class infoDialog : public QDialog
{
private:
    QLabel *m_imgFrame;
    QWidget *m_matrix;
    QPlainTextEdit *m_comment;
    QPlainTextEdit *m_lyrics;
    QPushButton *m_commentButton;
    QPushButton *m_lyricsButton;
    QWidget *m_extra;

private:
    infoDialog() {}
    infoDialog(const infoDialog&) = delete;
    infoDialog& operator=(const infoDialog&) = delete;

    void setDefaultImage() const;

public:
    infoDialog(QWidget*);
    ~infoDialog() override;

    void setInfo(const metaData*);
};

#endif
