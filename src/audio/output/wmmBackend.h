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

#ifndef WMM_H
#define WMM_H

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#ifdef _WIN32

#include <windows.h>
#include <mmsystem.h>

#include "outputBackend.h"

/*****************************************************************/

/**
 * WMM output backend
*/
class wmmBackend : public outputBackend
{
private:
    HWAVEOUT _wHandle;
    WAVEHDR _wHdr[2];
    HANDLE _event[2];
    unsigned int _idx;

private:
    static void CALLBACK waveOutProc(HWAVEOUT waveOut, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);
    static void errorMessage(const MMRESULT err);

private:
    wmmBackend();

public:
    virtual ~wmmBackend() {}

    static const char name[];

    /// Factory method
    static output* factory() { return new wmmBackend(); }

    /// Open
    size_t open(const unsigned int card, unsigned int &sampleRate, const unsigned int channels, const unsigned int prec) override;

    /// Close
    void close() override;

    /// Get buffer
    void *buffer() override { return _wHdr[_idx].lpData; }

    /// Write data to output
    bool write(void* buffer, size_t bufferSize) override;

    /// Pause
    void pause() override { stop(); }

    /// Stop
    void stop() override;

    /// Set volume
    void volume(int vol) override;

    /// Get volume
    int volume() const override;
};

#endif

#endif
