/*
 *  Copyright (C) 2010-2017 Leandro Nini
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

#ifndef EXIO_H
#define EXIO_H

#ifdef _WIN32
#  include <windows.h>
#  include <io.h>
#else
#  include <fcntl.h>
#  include <unistd.h>
#  include <errno.h>
#endif

#include <QFile>

class QFileEx : public QFile
{
    Q_OBJECT

private:
    QFileEx(const QFileEx&);
    QFileEx &operator=(const QFileEx&);

public:
    QFileEx(const QString & name) : QFile(name) {}

    bool lock(qint64 start, qint64 end)
    {
#ifdef _WIN32
        if (!LockFile((HANDLE)_get_osfhandle(handle()), (DWORD)start, (DWORD)(start>>32), (DWORD)end, (DWORD)(end>>32)))
            return false;
#else
        struct flock fl;

        fl.l_type=F_WRLCK;
        fl.l_whence=SEEK_SET;
        fl.l_start=start;
        fl.l_len=end-start;

        if (fcntl(handle(), F_SETLK, &fl)==-1)
            return false;
#endif
        return true;
    }
};

#endif
