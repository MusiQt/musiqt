/*
 *  Copyright (C) 2010-2019 Leandro Nini
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

#include <QtGlobal>

#if QT_VERSION >= 0x050100
#  include <QLockFile>
#else

#ifdef _WIN32
#  include <windows.h>
#  include <io.h>
#else
#  include <fcntl.h>
#  include <unistd.h>
#  include <errno.h>
#endif

#include <QFile>

class QLockFile
{
private:
    QLockFile(const QLockFile&);
    QLockFile &operator=(const QLockFile&);

private:
    QFile file;

    bool fileLocked;

public:
    QLockFile(const QString & name) : file(name), fileLocked(false) {}
    ~QLockFile() { file.close(); if (fileLocked) QFile::remove(file.fileName()); }

    bool tryLock()
    {
        if (file.open(QIODevice::ReadWrite))
        {
            const qint64 start = 0;
            const qint64 end = 0;
#ifdef _WIN32
            if (LockFile((HANDLE)_get_osfhandle(file.handle()), (DWORD)start, (DWORD)(start>>32), (DWORD)end, (DWORD)(end>>32)))
                fileLocked = true;
#else
            struct flock fl;

            fl.l_type = F_WRLCK;
            fl.l_whence = SEEK_SET;
            fl.l_start = start;
            fl.l_len = end-start;

            if (fcntl(file.handle(), F_SETLK, &fl) != -1)
                fileLocked = true;
#endif
        }

        return fileLocked;
    }

    void setStaleLockTime(int) {}
    void unlock() {}
};

#endif

#endif
