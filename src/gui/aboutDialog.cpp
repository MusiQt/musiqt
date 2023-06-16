/*
 *  Copyright (C) 2013-2023 Leandro Nini
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
#include <QApplication>

#include "xdg.h"

#define AUTHOR  "Leandro Nini"
#define YEAR    "2006-2023"

#define gettext(x) x

const char* infoString = gettext("<b>%1</b><br><br>Version %2<br><br>Copyright %3 %4<br><br>"
                          "Website: <a href=\"%5\">%5</a><br><br>Bugs: <a href=\"%6\">%6</a>");

const char* qtString = gettext("This program uses Qt version %1<br><br><a href=\"http://qt.io/\">http://qt.io/</a>");

aboutDialog::aboutDialog(QWidget *parent) :
    QDialog(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(QString(tr("About %1")).arg(PACKAGE_NAME));

    // Info
    QLabel *infoLabel = new QLabel(
        QString(tr(infoString)).arg(PACKAGE_NAME, PACKAGE_VERSION, YEAR, AUTHOR, PACKAGE_URL, PACKAGE_BUGREPORT),
        this
    );

    infoLabel->setOpenExternalLinks(true);

    QLabel *iconAppLabel = new QLabel(this);
    QPixmap iconAppLogo(":/icons/musiqt.png");
    iconAppLabel->setPixmap(iconAppLogo);

    QLabel* qtLabel = new QLabel(
        QString(tr(qtString)).arg(qVersion()),
        this
        );
    qtLabel->setAlignment(Qt::AlignCenter);
    qtLabel->setOpenExternalLinks(true);
    qtLabel->setTextFormat(Qt::RichText);

    QLabel *iconQtLabel = new QLabel(this);
    QPixmap qtLogo(":/icons/qt_icon32.png");
    iconQtLabel->setPixmap(qtLogo);

    QWidget *progWidget = new QWidget(this);
    QHBoxLayout *progLayout = new QHBoxLayout(progWidget);
    progLayout->addWidget(infoLabel);
    progLayout->addStretch(1);
    progLayout->addWidget(iconAppLabel);

    QWidget *qtWidget = new QWidget(this);
    QHBoxLayout *qtLayout = new QHBoxLayout(qtWidget);
    qtLayout->addWidget(iconQtLabel);
    qtLayout->addStretch(1);
    qtLayout->addWidget(qtLabel);

    QWidget *infoWidget = new QWidget(this);
    QVBoxLayout *infoLayout = new QVBoxLayout(infoWidget);
    infoLayout->addWidget(progWidget);
    infoLayout->addWidget(qtWidget);

    // Credits
    QLabel *creditsLabel = new QLabel(this);
    creditsLabel->setAlignment(Qt::AlignCenter);
    {
        QFile file(":/resources/CREDITS");
        file.open(QFile::ReadOnly|QFile::Text);
        QTextStream ts(&file);
#if QT_VERSION >= 0x060000
        ts.setEncoding(QStringConverter::Utf8);
#else
        ts.setCodec("UTF-8");
#endif
        creditsLabel->setText(ts.readAll());
    }
    QScrollArea* creditsArea = new QScrollArea(this);
    creditsArea->setWidget(creditsLabel);
    creditsArea->setAlignment(Qt::AlignCenter);

    // License
    QTextEdit *licenseLabel = new QTextEdit();
    licenseLabel->setReadOnly(true);
    {
        QFile file(":/resources/GPL_TEXT");
        file.open(QFile::ReadOnly|QFile::Text);
        QTextStream ts(&file);
        licenseLabel->setPlainText(ts.readAll());
    }

    // Log view
    QLabel *logLabel = new QLabel(this);
    {
        // FIXME this sucks
        QString stateDir = xdg::getStateDir();
        stateDir.append('/').append(qApp->organizationName());

        const QString logFileName(QString("%1/musiqt.log").arg(stateDir));
        QFile file(logFileName);
        file.open(QFile::ReadOnly|QFile::Text);
        QTextStream ts(&file);
#if QT_VERSION >= 0x060000
        ts.setEncoding(QStringConverter::Utf8);
#else
        ts.setCodec("UTF-8");
#endif
        logLabel->setText(ts.readAll());
    }
    QScrollArea* logArea = new QScrollArea(this);
    logArea->setWidget(logLabel);

    //
    QTabWidget *tabWidget = new QTabWidget(this);
    tabWidget->addTab(infoWidget, tr("&Info"));
    tabWidget->addTab(creditsArea, tr("C&redits"));
    tabWidget->addTab(licenseLabel, tr("&License"));
    tabWidget->addTab(logArea, tr("Lo&g"));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(this);
    QPushButton *close = buttonBox->addButton(QDialogButtonBox::Close);
    connect(close, &QPushButton::clicked, this, &aboutDialog::close);

    //QLabel *iconLabel = new QLabel(this);
    //iconLabel->setPixmap(QPixmap(":/icons/musiqt.png"));
    
    QWidget *bottomWidget = new QWidget(this);
    QHBoxLayout *bottomLayout = new QHBoxLayout(bottomWidget);
    //bottomLayout->addWidget(iconLabel);
    bottomLayout->addWidget(buttonBox);

    QVBoxLayout *vbox = new QVBoxLayout(this);
    vbox->addWidget(tabWidget);
    vbox->addWidget(bottomWidget);

    vbox->setContentsMargins(0,0,0,0);

    resize(minimumSize());

    layout()->setSizeConstraint(QLayout::SetFixedSize);
}
