/*
 *  Copyright (C) 2009-2021 Leandro Nini
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

configFrame::configFrame(QWidget* p, const char* credits, const char* link) :
    QWidget(p)
{
    QVBoxLayout* main = new QVBoxLayout();
    setLayout(main);

    if (credits != nullptr)
    {
        QFrame * creditBox = new QFrame();
        creditBox->setFrameStyle(QFrame::Box);
        main->addWidget(creditBox);

        QVBoxLayout* credLayout = new QVBoxLayout();
        creditBox->setLayout(credLayout);

        QLabel* cred = new QLabel();
        QString text = QString(credits);
        if (link != nullptr)
        {
            text.append("<br><br>").append(QString("<a href=\"%1\">%1</a>").arg(link));
            cred->setOpenExternalLinks(true);
        }
        cred->setText(text);
        cred->setTextFormat(Qt::RichText);
        cred->setAlignment(Qt::AlignCenter);
        credLayout->addWidget(cred);
    }

    QHBoxLayout* top = new QHBoxLayout();
    main->addLayout(top);

    m_matrix = new QGridLayout();
    top->addLayout(m_matrix);

    m_extraLeft = new QHBoxLayout();
    top->addLayout(m_extraLeft);

    m_extraBottom = new QVBoxLayout();
    main->addLayout(m_extraBottom);

    main->addStretch();
}
