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

#ifndef TIMEDISPLAY_H
#define TIMEDISPLAY_H

#include <QLCDNumber>

class timeDisplay final : public QLCDNumber
{
private:
    timeDisplay() {}
    timeDisplay(const timeDisplay&) = delete;
    timeDisplay& operator=(const timeDisplay&) = delete;

public:
    timeDisplay(QWidget* parent) :
        QLCDNumber(parent)
    {
        display("--:--");
        setSegmentStyle(QLCDNumber::Flat);
        setFrameStyle(QFrame::Panel|QFrame::Sunken);
    }
    ~timeDisplay() override = default;

    void reset() { display("--:--"); }

    void setTime(unsigned int seconds)
    {
        display(QString("%1:%2").arg(seconds/60, 2, 10, QChar('0')).arg(seconds%60, 2, 10, QChar('0')));
    }
};

/*****************************************************************/

#include <QLabel>

class timeLabel final : public QLabel
{
private:
    timeLabel() {}
    timeLabel(const timeLabel&) = delete;
    timeLabel& operator=(const timeLabel&) = delete;

public:
    timeLabel(QWidget* parent) :
        QLabel("--:--", parent)
    {
        setFrameStyle(QFrame::Panel|QFrame::Sunken);
    }
    ~timeLabel() override = default;

    void reset() { setText("--:--"); }

    void setTime(unsigned int seconds)
    {
        setText(QString("%1:%2").arg(seconds/60, 2, 10, QChar('0')).arg(seconds%60, 2, 10, QChar('0')));
    }
};

#endif
