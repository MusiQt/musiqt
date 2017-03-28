/*
 *  Copyright (C) 2009-2017 Leandro Nini
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

#include "configFrame.h"
#include "iconFactory.h"

#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>

configFrame::configFrame(QWidget* p, const char* title, const char* credits, const char* link) :
    QWidget(p)
{
    QVBoxLayout* main = new QVBoxLayout();
    setLayout(main);

    if (credits != nullptr)
    {
        QLabel* cred = new QLabel(credits, this);
        cred->setAlignment(Qt::AlignCenter);
        cred->setStyleSheet("background-color : white;");
        main->addWidget(cred);
    }

    if (link != nullptr)
    {
        QString text = QString("<a href=\"%1\">%1</a>").arg(link);
        QLabel* lnk = new QLabel(text, this);
        lnk->setAlignment(Qt::AlignCenter);
        lnk->setStyleSheet("background-color: white;");
        lnk->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
        lnk->setOpenExternalLinks(true);
        main->addWidget(lnk);
    }

    QHBoxLayout* top = new QHBoxLayout();
    main->addLayout(top);

    _matrix = new QGridLayout();
    top->addLayout(_matrix);

    _extraLeft = new QHBoxLayout();
    top->addLayout(_extraLeft);

    _extraBottom = new QVBoxLayout();
    main->addLayout(_extraBottom);
}
