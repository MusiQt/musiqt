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

#include "wmmBackend.h"

#include <QDebug>
#include <QThread>

#ifdef _WIN32

const char wmmBackend::name[]="WMM";

/*****************************************************************/

void CALLBACK wmmBackend::waveOutProc(HWAVEOUT waveOut, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
    if (uMsg == MM_WOM_DONE)
        SetEvent(*(HANDLE*)((WAVEHDR*)dwParam1)->dwUser);
}

/*****************************************************************/

wmmBackend::wmmBackend() :
    outputBackend(name)
{
    // Check devices
    WAVEOUTCAPS woc;
    const int n = waveOutGetNumDevs();

    for (int i=0; i<n; i++)
    {
        if (waveOutGetDevCaps(i, &woc, sizeof(WAVEOUTCAPS)) == MMSYSERR_NOERROR)
#ifdef UNICODE
        addDevice(QString::fromWCharArray(woc.szPname).toLocal8Bit().constData());
#else
        addDevice(woc.szPname);
#endif
    }
}

size_t wmmBackend::open(const unsigned int card, unsigned int &sampleRate, const unsigned int channels, const unsigned int prec)
{
    WAVEFORMATEX wfm;

    memset(&wfm, 0, sizeof(WAVEFORMATEX));

    wfm.wFormatTag = WAVE_FORMAT_PCM;
    wfm.nSamplesPerSec = sampleRate;
    wfm.nChannels = channels;
    wfm.wBitsPerSample = prec<<3;
    wfm.nBlockAlign = channels * prec;
    wfm.nAvgBytesPerSec = wfm.nSamplesPerSec * wfm.nBlockAlign;
    //wfm.cbSize=0;

    MMRESULT err=waveOutOpen(&_wHandle, card, &wfm, (DWORD_PTR)waveOutProc, 0, CALLBACK_FUNCTION);
    if (err != MMSYSERR_NOERROR)
    {
        qWarning() << "Error opening device " << device(card);
        errorMessage(err);
        return 0;
    }

    // Set buffer to contain 1 second of data
    const int bufferSize = wfm.nAvgBytesPerSec;
    qDebug() << "blocksize=" << bufferSize;

    _event[0] = CreateEvent(0, FALSE, FALSE, 0);
    _event[1] = CreateEvent(0, FALSE, FALSE, 0);

    memset(&_wHdr[0], 0, sizeof(WAVEHDR));
    _wHdr[0].lpData = new CHAR[bufferSize<<1];
    _wHdr[0].dwUser = (DWORD_PTR)&_event[0];

    memset(&_wHdr[1], 0, sizeof(WAVEHDR));
    _wHdr[1].lpData = _wHdr[0].lpData+bufferSize;
    _wHdr[1].dwUser = (DWORD_PTR)&_event[1];

    SetEvent(_event[1]);

    _idx = 0;

    return bufferSize;
}

void wmmBackend::close()
{
    CloseHandle(_event[0]);
    CloseHandle(_event[1]);

    while (waveOutUnprepareHeader(_wHandle, &_wHdr[1-_idx], sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
        QThread::usleep(10);

    waveOutClose(_wHandle);
    delete [] _wHdr[0].lpData;
}

void wmmBackend::errorMessage(const MMRESULT err)
{
    TCHAR buffer[MAXERRORLENGTH];
    waveOutGetErrorText(err, buffer, MAXERRORLENGTH);

#ifdef UNICODE
    qWarning() << QString::fromWCharArray(buffer);
#else
    qWarning() << buffer;
#endif
}

bool wmmBackend::write(void* buffer, size_t bufferSize)
{
    _wHdr[_idx].dwBufferLength = bufferSize;
    _wHdr[_idx].dwFlags = 0;

    MMRESULT err;

    err = waveOutPrepareHeader(_wHandle, &_wHdr[_idx], sizeof(WAVEHDR));
    if (err != MMSYSERR_NOERROR)
    {
        qWarning() << "waveOutPrepareHeader:";
        errorMessage(err);
        return false;
    }

    ResetEvent(_event[_idx]);

    err = waveOutWrite(_wHandle, &_wHdr[_idx], sizeof(WAVEHDR));
    if (err != MMSYSERR_NOERROR)
    {
        qWarning() << "waveOutWrite:";
        errorMessage(err);
        return false;
    }

    _idx = 1-_idx;

    if (WaitForSingleObject(_event[_idx], INFINITE) != WAIT_OBJECT_0)
    {
        qWarning() << "Error waiting for sound to finish";
        return false;
    }

    err = waveOutUnprepareHeader(_wHandle, &_wHdr[_idx], sizeof(WAVEHDR));
    if (err != MMSYSERR_NOERROR)
    {
        qWarning() << "waveOutUnprepareHeader:";
        errorMessage(err);
        return false;
    }

    return true;
}

void wmmBackend::stop()
{
    waveOutReset(_wHandle);
}

void wmmBackend::volume(int vol)
{
    vol = (vol*65535)/100;
    vol |= (vol<<16);
    waveOutSetVolume(_wHandle, vol);
}

int wmmBackend::volume() const
{
    DWORD vol;
    waveOutGetVolume(_wHandle, &vol);
    return (int)(((vol&0xFFFF)*100)/65535);
}

#endif
