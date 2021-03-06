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

#ifndef CONFIG_FRAME_H
#define CONFIG_FRAME_H

#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>

class configFrame : public QWidget
{
private:
    QGridLayout *m_matrix;
    QHBoxLayout *m_extraLeft;
    QVBoxLayout *m_extraBottom;

private:
    configFrame(const configFrame&);
    configFrame& operator=(const configFrame&);

protected:
    configFrame() {}

    QGridLayout* matrix() const { return m_matrix; }
    QHBoxLayout* extraLeft() const { return m_extraLeft; }
    QVBoxLayout* extraBottom() const { return m_extraBottom; }

public:
    configFrame(QWidget* p, const char* title, const char* credits=nullptr, const char* link=nullptr);
    virtual ~configFrame() {}
};

#endif
