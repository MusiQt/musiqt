## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign

SUBDIRS = po

DISTCHECK_CONFIGURE_FLAGS = \
--enable-hvl-plugin

icondir = $(prefix)/share/icons
icon_DATA = musiqt.png

shelldesktopdir = $(prefix)/share/applications
shelldesktop_DATA = musiqt.desktop

if MINGW32
resfile = src/resfile.o

src/resfile.o: src/resfile.rc
	@target_alias@-windres $< -o $@
endif

EXTRA_DIST = \
musiqt.png \
src/resources.qrc \
src/icons \
src/resources \
src/libs/hvl_replay/ChangeLog.txt \
src/libs/unzip/MiniZip64_info.txt \
src/gui/ledGreen.xpm \
src/gui/ledRed.xpm \
src/gui/ledYellow.xpm

if ZLIB_SUPPORT
ZLIB_CPPFLAGS = -DNOUNCRYPT

if MINGW32
ZLIBEXTRASOURCES = \
src/libs/unzip/iowin32.h \
src/libs/unzip/iowin32.c
endif

ZLIBSOURCES = \
src/libs/unzip/ioapi.h \
src/libs/unzip/ioapi.c \
$(ZLIBEXTRASOURCES) \
src/libs/unzip/unzip.h \
src/libs/unzip/unzip.c

endif

if MPG123
MPG123_SOURCES = \
src/audio/input/mpg123Backend.cpp \
src/audio/input/mpg123Backend.h
MPG123_MOC = \
src/audio/input/mpg123Backend_moc.cpp
endif

if FFMPEG
FFMPEG_SOURCES = \
src/audio/input/ffmpegBackend.cpp \
src/audio/input/ffmpegBackend.h
FFMPEG_MOC = \
src/audio/input/ffmpegBackend_moc.cpp
endif

if VORBIS
VORBIS_SOURCES = \
src/audio/input/oggBackend.cpp \
src/audio/input/oggBackend.h
VORBIS_MOC = \
src/audio/input/oggBackend_moc.cpp
endif

if OPUS
OPUS_SOURCES = \
src/audio/input/opusBackend.cpp \
src/audio/input/opusBackend.h
OPUS_MOC = \
src/audio/input/opusBackend_moc.cpp
endif

if GME
GME_SOURCES =\
src/audio/input/gmeBackend.h \
src/audio/input/gmeBackend.cpp
GME_MOC = \
src/audio/input/gmeBackend_moc.cpp
endif

if HVL
HVLSOURCES = \
src/libs/hvl_replay/hvl_replay.h \
src/libs/hvl_replay/hvl_replay.c \
src/libs/hvl_replay/hvl_tables.h \
src/libs/hvl_replay/hvl_tables.c 
HVL_SOURCES = \
src/audio/input/hvlBackend.h \
src/audio/input/hvlBackend.cpp
HVL_MOC = \
src/audio/input/hvlBackend_moc.cpp
endif

if SIDPLAYFP
SIDPLAYFP_SOURCES = \
src/audio/input/sidBackend.cpp \
src/audio/input/sidBackend.h
SIDPLAYFP_MOC = \
src/audio/input/sidBackend_moc.cpp
endif

if OPENMPT
OPENMPT_SOURCES = \
src/audio/input/openmptBackend.cpp \
src/audio/input/openmptBackend.h
OPENMPT_MOC = \
src/audio/input/openmptBackend_moc.cpp
endif

if SNDFILE
SNDFILE_SOURCES = \
src/audio/input/sndBackend.cpp \
src/audio/input/sndBackend.h
SNDFILE_MOC = \
src/audio/input/sndBackend_moc.cpp
endif

if MPC
MPC_SOURCES = \
src/audio/input/mpcBackend.cpp \
src/audio/input/mpcBackend.h
MPC_MOC = \
src/audio/input/mpcBackend_moc.cpp
endif

if WAVPACK
WAVPACK_SOURCES = \
src/audio/input/wvBackend.cpp \
src/audio/input/wvBackend.h
WAVPACK_MOC = \
src/audio/input/wvBackend_moc.cpp
endif

bin_PROGRAMS = src/musiqt

src_musiqt_CPPFLAGS = \
@debug_flags@ \
@FF_FLAGS@ \
-I$(top_srcdir)/src \
-I$(top_srcdir)/src/audio \
-I$(top_srcdir)/src/gui \
-I$(top_srcdir)/src/trackList \
-I$(top_srcdir)/src/utils \
-I$(top_builddir)/src/utils \
$(QT_CFLAGS) \
$(BS2B_CFLAGS) \
$(OPENMPT_CFLAGS) \
$(VORBIS_CFLAGS) \
$(MPG123_CFLAGS) \
$(OPUS_CFLAGS) \
$(GME_CFLAGS) \
$(SIDPLAYFP_CFLAGS) \
$(STILVIEW_CFLAGS) \
$(SNDFILE_CFLAGS) \
$(WAVPACK_CFLAGS) \
$(ZLIB_CPPFLAGS) \
-DLOCALEDIR=\"$(localedir)\"

src_musiqt_SOURCES = \
src/gettext.h \
src/main.cpp \
src/singleApp.cpp \
src/singleApp.h \
src/translator.cpp \
src/translator.h \
src/audio/audio.cpp \
src/audio/audio.h \
src/audio/configFrame.cpp \
src/audio/configFrame.h \
src/audio/converter/converter.h \
src/audio/converter/converters.cpp \
src/audio/converter/converters.h \
src/audio/converter/converterBackend.cpp \
src/audio/converter/converterBackend.h \
src/audio/converter/converterFactory.cpp \
src/audio/converter/converterFactory.h \
src/audio/converter/quantizer.cpp \
src/audio/converter/quantizer.h \
src/audio/inputTypes.h \
src/audio/input/input.h \
src/audio/input/inputBackend.cpp \
src/audio/input/inputBackend.h \
src/audio/input/inputFactory.cpp \
src/audio/input/inputFactory.h \
src/audio/input/metaData.cpp \
src/audio/input/metaData.h \
$(MPG123_SOURCES) \
$(FFMPEG_SOURCES) \
$(OPENMPT_SOURCES) \
$(VORBIS_SOURCES) \
$(OPUS_SOURCES) \
$(GME_SOURCES) \
$(HVL_SOURCES) \
${SIDPLAYFP_SOURCES} \
$(SNDFILE_SOURCES) \
$(MPC_SOURCES) \
$(WAVPACK_SOURCES) \
src/audio/output/output.h \
src/audio/output/outputBackend.h \
src/audio/output/outputFactory.cpp \
src/audio/output/outputFactory.h \
src/audio/output/qaudioBackend.cpp \
src/audio/output/qaudioBackend.h \
src/gui/aboutDialog.cpp \
src/gui/aboutDialog.h \
src/gui/bookmark.cpp \
src/gui/bookmark.h \
src/gui/centralFrame.cpp \
src/gui/centralFrame.h \
src/gui/iconFactory.cpp \
src/gui/iconFactory.h \
src/gui/icons.h \
src/gui/infoDialog.cpp \
src/gui/infoDialog.h \
src/gui/infoLabel.cpp \
src/gui/infoLabel.h \
src/gui/mainWindow.cpp \
src/gui/mainWindow.h \
src/gui/playlistModel.h \
src/gui/settings.cpp \
src/gui/settings.h \
src/gui/timeDisplay.h \
src/trackList/trackListBackend.h \
src/trackList/trackListDir.h \
src/trackList/trackListFactory.cpp \
src/trackList/trackListFactory.h \
src/trackList/trackList.h \
src/trackList/trackListM3u.h \
src/trackList/trackListPls.h \
src/trackList/trackListCue.h \
$(ZLIBSOURCES) \
$(STILVIEWSOURCES) \
$(HVLSOURCES) \
src/utils/AutoDLL.h \
src/utils/AutoDLL.cpp \
src/utils/genres.h \
src/utils/ioEx.h \
src/utils/loadsym.h \
src/utils/utils.cpp \
src/utils/utils.h \
src/utils/tag.cpp \
src/utils/tag.h \
src/utils/xdg.cpp \
src/utils/xdg.h

nodist_src_musiqt_SOURCES = \
src/singleApp_moc.cpp \
src/translator_moc.cpp \
src/audio/audio_moc.cpp \
src/audio/configFrame_moc.cpp \
$(MPG123_MOC) \
$(FFMPEG_MOC) \
$(OPENMPT_MOC) \
$(VORBIS_MOC) \
$(OPUS_MOC) \
$(GME_MOC) \
$(HVL_MOC) \
$(SIDPLAYFP_MOC) \
$(SNDFILE_MOC) \
$(MPC_MOC) \
$(WAVPACK_MOC) \
src/gui/aboutDialog_moc.cpp \
src/gui/bookmark_moc.cpp \
src/gui/centralFrame_moc.cpp \
src/gui/infoDialog_moc.cpp \
src/gui/infoLabel_moc.cpp \
src/gui/mainWindow_moc.cpp \
src/gui/playlistModel_moc.cpp \
src/gui/settings_moc.cpp \
src/gui/timeDisplay_moc.cpp \
src/resources_rcc.cpp

src_musiqt_LDADD = $(resfile) \
$(QT_LIBS) \
$(AUDIO_LIBS) \
$(BS2B_LIBS) \
$(OPENMPT_LIBS) \
$(VORBIS_LIBS) \
$(MPG123_LIBS) \
$(OPUS_LIBS) \
$(GME_LIBS) \
$(SIDPLAYFP_LIBS) \
$(STILVIEW_LIBS) \
$(SNDFILE_LIBS) \
$(WAVPACK_LIBS) \
@LIBINTL@


#BUILT_SOURCES =

CLEANFILES = \
$(nodist_src_musiqt_SOURCES) \
$(resfile)

%_ui.h: %.ui
	${UIC} $< -o $@

%_moc.cpp: %.h
	${MOC} $< -o $@

%_rcc.cpp: %.qrc
	${RCC} $< -o $@
