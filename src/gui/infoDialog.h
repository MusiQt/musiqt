/*
 *  Copyright (C) 2006-2017 Leandro Nini
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
#include <QGridLayout>
#include <QImage>
#include <QLabel>
#include <QThread>
#include <QPlainTextEdit>

class metaData;

class imageLoader : public QThread
{
    Q_OBJECT

private:
    QString _name;

protected:
    void run() override;

public:
    imageLoader()
        : QThread() {}

    /// Load an image
    void load(const QString& file);

signals:
    void loaded(const QImage* image);
};

/*****************************************************************/

class infoDialog : public QDialog
{
    Q_OBJECT

private:
    imageLoader *_imageLoader;
    QLabel *_imgFrame;
    QGridLayout *_matrix;
    QPlainTextEdit *_text;

private:
    infoDialog() {}
    infoDialog(const infoDialog&);
    infoDialog& operator=(const infoDialog&);

public:
    infoDialog(QWidget*);
    virtual ~infoDialog();

    void setInfo(const metaData*);

public slots:
    void onImgLoaded(const QImage*);
};

#endif
