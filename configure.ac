# Process this file with autoconf to create configure.

AC_INIT([musiqt],[0.3.0],[https://github.com/drfiemost/musiqt/issues/],[musiqt],[https://github.com/drfiemost/musiqt/])
AC_CONFIG_SRCDIR([src/main.cpp])
AC_CONFIG_HEADERS([src/config.h])
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([subdir-objects])
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.19.8])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CPP

AX_CXX_COMPILE_STDCXX_11([noext],[mandatory])


# Checks for languages.
AC_LANG([C++])

AC_CANONICAL_HOST()
AS_CASE([$host_os],
        [mingw*], [MINGW32=yes],
        [MINGW32=no]
)
AM_CONDITIONAL([MINGW32], [test "x$MINGW32" = "xyes"])

#####################################################
# Check for debug build
#####################################################
AC_MSG_CHECKING([for debugging])
AC_ARG_ENABLE(debug, [AS_HELP_STRING([--enable-debug], [compile for debugging])])
AC_MSG_RESULT([$enable_debug])

AS_IF([test "x$enable_debug" = "xyes"],
      [debug_flags="-g -Wall -O0 -DDEBUG"
        CXXFLAGS=""
        CFLAGS=""
        AS_IF([test "x$MINGW32" = "xyes"],
              [LDFLAGS="${LDFLAGS} -mconsole"],
              [LDFLAGS=""]
        )],
        [debug_flags="-DNDEBUG -DQT_NO_DEBUG_OUTPUT"]
)

AC_SUBST([debug_flags])


#####################################################
# Check for audio libs
#####################################################

##################################################################
# Search for OSS

AC_CHECK_HEADERS([sys/soundcard.h machine/soundcard.h], [break])

##################################################################
# Search for ALSA

PKG_CHECK_MODULES(ALSA,
        [alsa >= 1.0],
        [AC_DEFINE([HAVE_ALSA], 1, [Define to 1 if you have libasound (-lasound).])],
        [AC_MSG_WARN([$ALSA_PKG_ERRORS])]
)

##################################################################
# Search for Windows audio API

AUDIO_LDFLAGS=""

AC_CHECK_HEADERS([mmsystem.h], [], [], [#include <windows.h>])

AS_IF([test "$ac_cv_header_mmsystem_h" = "yes"],
    [AUDIO_LDFLAGS="$AUDIO_LDFLAGS -lwinmm"]
)

AC_SUBST([AUDIO_LDFLAGS])

##################################################################
# Search for zlib

AC_CHECK_HEADER([zlib.h], [AC_CHECK_LIB(z, inflate)])

AM_CONDITIONAL([ZLIB_SUPPORT], [ test "x${ac_cv_lib_z_inflate}" = xyes ])

##################################################################
# Search for libsndfile

AC_ARG_ENABLE([sndfile-plugin], [AS_HELP_STRING([--disable-sndfile-plugin], [disable sndfile backend])])

AS_IF([test "x$enable_sndfile_plugin" != xno],
    [AS_IF([test "x$MINGW32" = "xyes"],
        [PKG_CHECK_EXISTS([sndfile >= 1.0.21],
            [AC_DEFINE([ENABLE_SNDFILE_WINDOWS_PROTOTYPES], 1, [Define to 1 if you have libsndfile>=1.0.21 on MinGW.])]
        )]
    )
    PKG_CHECK_MODULES(SNDFILE,
        [sndfile >= 1.0],
        [AC_DEFINE([HAVE_SNDFILE], 1, [Define to 1 if you have libsndfile (-lsndfile).])],
        [AC_MSG_WARN([$SNDFILE_PKG_ERRORS])]
    )]
)
AM_CONDITIONAL([SNDFILE], [test "x$pkg_cv_SNDFILE_LIBS" != "x"])

##################################################################
# Search for libmpg123

AC_ARG_ENABLE([mpg123-plugin], [AS_HELP_STRING([--disable-mpg123-plugin], [disable mpg123 backend])])

AS_IF([test "x$enable_mpg123_plugin" != xno],
    [PKG_CHECK_MODULES(MPG123,
        [libmpg123 >= 1.6],
        [AC_DEFINE([HAVE_MPG123], 1, [Define to 1 if you have libmpg123 (-lmpg123).])],
        [AC_MSG_WARN([$MPG123_PKG_ERRORS])]
    )]
)
AM_CONDITIONAL([MPG123], [test "x$pkg_cv_MPG123_LIBS" != "x"])

##################################################################
# Search for ffmpeg

AC_CHECK_HEADERS([libavformat/avformat.h], [break], [], [#define __STDC_CONSTANT_MACROS])

AS_IF([ test "x${ac_cv_header_libavformat_avformat_h}" = xyes ],
    [FF_FLAGS="-D__STDC_CONSTANT_MACROS"]
)

AC_SUBST([FF_FLAGS])
AM_CONDITIONAL([FFMPEG], [test "x$ac_cv_header_libavformat_avformat_h" = "xyes"])

##################################################################
# Search for vorbisfile

AC_ARG_ENABLE([vorbis-plugin], [AS_HELP_STRING([--disable-vorbis-plugin], [disable ogg-vorbis backend])])

AS_IF([test "x$enable_vorbis_plugin" != xno],
    [PKG_CHECK_MODULES(VORBIS,
        [vorbisfile >= 1.1],
        [AC_DEFINE([HAVE_VORBIS], 1, [Define to 1 if you have libvorbisfile (-lvorbisfile).])],
        [AC_MSG_WARN([$VORBIS_PKG_ERRORS])]
    )]
)
AM_CONDITIONAL([VORBIS], [test "x$pkg_cv_VORBIS_LIBS" != "x"])

##################################################################
# Search for opusfile

AC_ARG_ENABLE([opus-plugin], [AS_HELP_STRING([--disable-opus-plugin], [disable opus backend])])

AS_IF([test "x$enable_opus_plugin" != xno],
    [PKG_CHECK_MODULES(OPUS,
        [opusfile >= 0.7],
        [AC_DEFINE([HAVE_OPUS], 1, [Define to 1 if you have libopusfile (-lopusfile).])],
        [AC_MSG_WARN([$OPUS_PKG_ERRORS])]
    )]
)
AM_CONDITIONAL([OPUS], [test "x$pkg_cv_OPUS_LIBS" != "x"])

##################################################################
# Check if we should build Game Music Engine backend

AC_ARG_ENABLE([gme-plugin], [AS_HELP_STRING([--disable-gme-plugin], [disable Game_Music_Emu backend])])

AS_IF([test "x$enable_gme_plugin" != xno],
    [PKG_CHECK_MODULES(GME,
        [libgme >= 0.6],
        [AC_DEFINE([HAVE_GME], 1, [Define to 1 if you have libgme (-lgme).])],
        [AC_MSG_WARN([$GME_PKG_ERRORS])]
    )]
)
AM_CONDITIONAL([GME], [test "x$pkg_cv_GME_LIBS" != "x"])

##################################################################
# Check if we should build Hively backend

AC_ARG_ENABLE([hvl-plugin], [AS_HELP_STRING([--enable-hvl-plugin], [enable Hively backend])])

AS_IF([test "x$enable_hvl_plugin" = xyes],
    [AC_DEFINE([BUILD_HVL], 1, [Define to 1 if you want the Hively backend.])]
)
AM_CONDITIONAL([HVL], [test "x$enable_hvl_plugin" = xyes])

##################################################################
# Search for libsidplayfp

AC_ARG_ENABLE([sidplayfp-plugin], [AS_HELP_STRING([--disable-sidplayfp-plugin], [disable sidplayfp backend])])

AS_IF([test "x$enable_sidplayfp_plugin" != xno],
    [PKG_CHECK_MODULES(SIDPLAYFP,
        [libsidplayfp >= 1.0],
        [AC_DEFINE([HAVE_SIDPLAYFP], 1, [Define to 1 if you have libsidplayfp (-lsidplayfp).])
        AC_CHECK_HEADERS([sidplayfp/builders/residfp.h \
            sidplayfp/builders/resid.h \
            sidplayfp/builders/hardsid.h \
            sidplayfp/builders/exsid.h])
        ],
        [AC_MSG_WARN([$SIDPLAYFP_PKG_ERRORS])]
    )]
)
AM_CONDITIONAL([SIDPLAYFP], [test "x$pkg_cv_SIDPLAYFP_LIBS" != "x"])

##################################################################
# Search for libopenmpt

AC_ARG_ENABLE([openmpt-plugin], [AS_HELP_STRING([--disable-openmpt-plugin], [disable openmpt backend])])

AS_IF([test "x$enable_openmpt_plugin" != xno -a "x$HAVE_CXX11" = "x1"],
    [PKG_CHECK_MODULES(OPENMPT,
        [libopenmpt >= 0.2],
        [AC_DEFINE([HAVE_OPENMPT], 1, [Define to 1 if you have libopenmpt (-lopenmp).])],
        [AC_MSG_WARN([$OPENMPT_PKG_ERRORS])]
    )]
)
AM_CONDITIONAL([OPENMPT], [test "x$pkg_cv_OPENMPT_LIBS" != "x"])

##################################################################
# Search for libmpc

AC_ARG_ENABLE([mpc-plugin], [AS_HELP_STRING([--disable-mpc-plugin], [disable musepack backend])])

AS_IF([test "x$enable_mpc_plugin" != xno],
    [AC_CHECK_HEADERS([mpc/mpcdec.h mpcdec/mpcdec.h], [break])
    AS_IF([test "x${ac_cv_header_mpc_mpcdec_h}" = xyes],
        [AC_CHECK_LIB(mpcdec, [mpc_demux_init])],
        [test "x${ac_cv_header_mpcdec_mpcdec_h}" = xyes],
        [AC_CHECK_LIB(mpcdec, [mpc_decoder_initialize])]
    )]
)
AM_CONDITIONAL([MPC], [test "x$ac_cv_lib_mpcdec_mpc_decoder_initialize" = "xyes" \
    -o "x$ac_cv_lib_mpcdec_mpc_demux_init" = "xyes"])

##################################################################
# Search for wavpack

PKG_CHECK_MODULES(WAVPACK,
    [wavpack >= 4],
    [AC_DEFINE([HAVE_WAVPACK], 1, [Define to 1 if you have wavpack (-lwavpack).])],
    [AC_MSG_WARN([$WAVPACK_PKG_ERRORS])]
)
AM_CONDITIONAL([WAVPACK], [test "x$pkg_cv_WAVPACK_LIBS" != "x"])

##################################################################
# Check if we need STIL support

AS_IF([test "x$enable_gme_plugin" = xyes -o "x$enable_sidplayfp_plugin" != xno],
    [PKG_CHECK_MODULES(STILVIEW,
        [libstilview >= 1.0],
        [AC_DEFINE([HAVE_STILVIEW], 1, [Define to 1 if you have libstilview (-lstilview).])],
        [AC_MSG_WARN([$STILVIEW_PKG_ERRORS])]
    )
    AS_IF([test "x$pkg_cv_STILVIEW_LIBS" = "x"],
        internal_stil=yes
    )]
)

##################################################################
# Search for bs2b

PKG_CHECK_MODULES(BS2B,
    [libbs2b >= 2.2],
    [AC_DEFINE([HAVE_BS2B], 1, [Define to 1 if you have libbs2b (-lbs2b).])],
    [AC_MSG_WARN([$BS2B_PKG_ERRORS])]
)

#####################################################
# Check for Qt
#####################################################
PKG_CHECK_MODULES(QT, [Qt5Core Qt5Gui Qt5Network Qt5Widgets Qt5Multimedia >= 5.6.0], [
  QT5_PATH="$( eval $PKG_CONFIG --variable=host_bins Qt5Core )"
  AX_APPEND_FLAG([-fPIC])
  AC_PATH_PROGS(MOC, [moc-qt5 moc], [moc], [$QT5_PATH])
  AC_PATH_PROGS(RCC, [rcc-qt5 rcc], [rcc], [$QT5_PATH])
  AC_PATH_PROGS(UIC, [uic-qt5 uic], [uic], [$QT5_PATH])
],
  [PKG_CHECK_MODULES(QT, [QtCore QtGui QtNetwork QtMultimedia >= 4.6.0], [
    QT4_PATH="$( eval $PKG_CONFIG --variable=exec_prefix QtCore )/bin"
    AC_PATH_PROGS(MOC, [moc-qt4 moc], [moc], [$QT4_PATH])
    AC_PATH_PROGS(RCC, [rcc-qt4 rcc], [rcc], [$QT4_PATH])
    AC_PATH_PROGS(UIC, [uic-qt4 uic], [uic], [$QT4_PATH])
  ])]
)

AC_CONFIG_FILES([
po/Makefile.in
Makefile
musiqt.desktop
src/resfile.rc
])

AC_OUTPUT
