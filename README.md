[![CI](https://github.com/MusiQt/musiqt/actions/workflows/build.yml/badge.svg)](https://github.com/MusiQt/musiqt/actions/workflows/build.yml)
[![](https://img.shields.io/github/downloads/MusiQt/musiqt/latest/total.svg)](https://github.com/MusiQt/musiqt/releases/latest)

MusiQt - https://github.com/MusiQt/musiqt/  
Copyright (C) 2006-2025 Leandro Nini (drfiemost@users.sourceforge.net)

![splash](https://github.com/MusiQt/musiqt/wiki/images/splash.jpg)

********************************************************************

MusiQt is a simple and cross-platform audio player.

It features a dirlist view, where you can browse through your disk and
bookmark directories in which you've stored your music, and a playlist
view which is automatically filled with all the supported files
contained in the selected directory. Then you can select whether to play
the whole playlist or only a single song.
It is also possible to create custom playlist and save them for later use,
just enter the edit mode and drag files from the dirlist view to the playlist.
It supports gapless playback, MPRIS and Last.fm scrobbling.

Compact view:

![screenshot compact view](https://github.com/MusiQt/musiqt/wiki/images/screenshot_compact.jpg)

Full view:

![screenshot full view](https://github.com/MusiQt/musiqt/wiki/images/screenshot_full.jpg)

********************************************************************

Formats currently supported:

__Openmpt__
  - mod, s3m, xm, it, 669, amf, ams, dbm, dmf, dsm, far, mdl, med, mtm, okt, ptm, stm, ult, umx, mt2, psm, mo3
  - xmz, itz, mdz, s3z (if zlib is installed)

__Sidplayfp__
  - sid, mus, prg, p00 (includes songlength DB and STIL support, str and wds support for mus)

__Ogg-Vorbis__
  - ogg, oga

__Opus__
  - opus

__Sndfile__
  - wav, voc, au, aiff, aifc
  - flac, caf (if libsndfile>=1.0.12)
  - vox (if libsndfile>=1.0.2)

__Ffmpeg__
  - aac, wma, au, flac, mp2, mp3, m2a, mpc, ogg, rm, ra, ram, tta, voc, wav, wv, ape
(depends on ffmpeg version)

__Gme__
  - ay, gbs, gym, hes, kss, nsfe, nsf, sap, spc, vgm
  - sap (includes STIL support)
  - vgz (if zlib is installed)

__Musepack__
  - mpc

__WavPack__
  - wv

__Hively__
  - ahx, hvl

__Mpg123__
  - mp3

*Note*:
ffmpeg backend is enabled only when needed libraries are found
at runtime, so you need to install them on your own.
Beware that these libraries may contain patented code and support some
proprietary codec.

Some Linux distro already ships these libraries.
(libavcodec.so, libavformat.so and libavutil.so)

On Windows you have to manually put them under the System folder
or in the same folder as musiqt binary.
(avcodec-58.dll, avformat-58.dll and avutil-56.dll)

********************************************************************

*Required dependencies*:

A compiler with c++17 support

Qt 5.15 or >= 6.2  
http://www.qt.io/


*Optional dependencies*:

libopenmpt >= 0.3  
http://lib.openmpt.org/libopenmpt/

zlib 1.2.x  
http://www.zlib.org/

libsidplayfp >= 1.0  
https://github.com/libsidplayfp/libsidplayfp/

libvorbisfile >= 1.1.x  
http://www.xiph.org/

libopusfile >= 0.7  
http://opus-codec.org/

libsndfile 1.0.x  
https://github.com/libsndfile/libsndfile

libmpcdec 1.2.x (SV7)
or
libmpcs 0.1 (SV8)  
http://www.musepack.com/

WavPack >= 4.x  
http://www.wavpack.com/

libmpg123 >= 1.6.x  
http://mpg123.org/

game-music-emu >= 0.6  
https://bitbucket.org/mpyne/game-music-emu/

libbs2b 3.x  
http://bs2b.sourceforge.net/

ffmpeg >= 4.0  
http://www.ffmpeg.org/
  - libavcodec >= 58
  - libavformat >= 58.12
  - avutil >= 56

********************************************************************

*Build*:
```
git clone https://github.com/MusiQt/musiqt.git
cd musiqt
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```
The following build options are available:
~~~
-DQT6=OFF :       enable building with Qt5 if found
-DPORTABLE=ON :   only for Windows, build a portable app
-DNLS=OFF :       disable Native Language Support
-DLTO=OFF :       disable link time optimization
-DLASTFM=OFF :    disable Last.fm scrobbling

-DSNDFILE=OFF :   disable Sndfile backend
-DMPG123=OFF :    disable mpg123 backend
-DVORBIS=OFF :    disable Ogg-Vorbis backend
-DOPUS=OFF :      disable Opus backend
-DGME=OFF :       disable Game Music Engine backend
-DOPENMPT=OFF :   disable OpenMpt backend
-DWAVPACK=OFF :   disable Wavpack backend
-DSIDPLAYFP=OFF : disable sidplayfp backend
-DHVL=OFF :       disable Hively backend
-DMPC=OFF :       disable Musepack backend
~~~
********************************************************************
