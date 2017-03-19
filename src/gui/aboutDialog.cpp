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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "aboutDialog.h"

#include "iconFactory.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QLabel>
#include <QTabWidget>
#include <QTextEdit>
#include <QTextStream>
#include <QFile>
#include <QScrollArea>

#define AUTHOR  "Leandro Nini"
#define WEBSITE "https://github.com/drfiemost/musiqt/"
#define YEAR    "2006-2017"

aboutDialog::aboutDialog(QWidget *parent) :
    QDialog(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(QString(tr("About %1")).arg(PACKAGE_NAME));

    // Info
    QPixmap iconAppLogo(":/icons/logo32.gif");
    QLabel *infoLabel = new QLabel(QString(tr("%1\n\nVersion %2\n\nCopyright %3 %4")).arg(PACKAGE_NAME, PACKAGE_VERSION, YEAR, AUTHOR));
    QLabel *iconAppLabel = new QLabel();
    iconAppLabel->setPixmap(iconAppLogo);

    QWidget *infoWidget = new QWidget();
    QHBoxLayout *infoLayout = new QHBoxLayout(infoWidget);
    infoLayout->addWidget(infoLabel);
    infoLayout->addStretch(1);
    infoLayout->addWidget(iconAppLabel);

    // Contact
    QLabel *contactLabel = new QLabel(QString(tr("Bugs: <a href=\"%1\">%1</a>")).arg(PACKAGE_BUGREPORT));
    contactLabel->setOpenExternalLinks(true);
    QLabel *websiteLabel = new QLabel(QString(tr("<a href=\"%1\">%1</a>")).arg(WEBSITE));
    websiteLabel->setOpenExternalLinks(true);

    QWidget *contactWidget = new QWidget();
    QVBoxLayout *contactLayout = new QVBoxLayout(contactWidget);
    contactLayout->addWidget(websiteLabel);
    contactLayout->addWidget(contactLabel);

    // Credits
    QLabel *creditsLabel = new QLabel();
    creditsLabel->setAlignment(Qt::AlignCenter);
    {
        QFile file(":/resources/CREDITS");
        file.open(QFile::ReadOnly|QFile::Text);
        QTextStream ts(&file);
        creditsLabel->setText(ts.readAll());
    }
    QScrollArea* creditsArea = new QScrollArea(this);
    creditsArea->setWidget(creditsLabel);
    creditsArea->setAlignment(Qt::AlignCenter);

    // Qt
    QLabel* qtWidget = new QLabel(/*GET_ICON(icon_qtbig), */
        QString("This program uses Qt version %1<br><br><a href=\"http://qt.io/\">http://qt.io/</a>")
            .arg(qVersion())
        );
    qtWidget->setAlignment(Qt::AlignCenter);
    qtWidget->setOpenExternalLinks(true);
    qtWidget->setTextFormat(Qt::RichText);

    // License
    QTextEdit *licenseLabel = new QTextEdit();
    licenseLabel->setReadOnly(true);
    {
        QFile file(":/resources/GPL_TEXT");
        file.open(QFile::ReadOnly|QFile::Text);
        QTextStream ts(&file);
        licenseLabel->setPlainText(ts.readAll());
    }

    QTabWidget *tabWidget = new QTabWidget();
    tabWidget->addTab(infoWidget, tr("&Info"));
    tabWidget->addTab(contactWidget, tr("&Contact"));
    tabWidget->addTab(creditsArea, tr("&Credits"));
    tabWidget->addTab(qtWidget, tr("&Qt"));
    tabWidget->addTab(licenseLabel, tr("&License"));

    QDialogButtonBox *buttonBox = new QDialogButtonBox();
    QPushButton *close = buttonBox->addButton(QDialogButtonBox::Close);
    connect(close, SIGNAL(clicked()), this, SLOT(close()));

    QLabel *iconLabel = new QLabel();
    iconLabel->setPixmap(QPixmap(":/icons/drf.gif"));
    
    QWidget *bottomWidget = new QWidget();
    QHBoxLayout *bottomLayout = new QHBoxLayout(bottomWidget);
    bottomLayout->addWidget(iconLabel);
    bottomLayout->addWidget(buttonBox);

    QVBoxLayout *vbox = new QVBoxLayout(this);
    vbox->addWidget(tabWidget);
    vbox->addWidget(bottomWidget);

    vbox->setContentsMargins(0,0,0,0);

    resize(minimumSize());
}
