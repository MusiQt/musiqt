cmake_minimum_required(VERSION 3.12)
project(musiqt VERSION 0.4.0 HOMEPAGE_URL https://github.com/drfiemost/musiqt/)

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/src/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/src/config.h)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(MUSIQT_SOURCES
    src/resources.qrc
    src/gettext.h
    src/main.cpp
    src/singleApp.cpp
    src/singleApp.h
    src/translator.cpp
    src/translator.h
    src/audio/audio.cpp
    src/audio/audio.h
    src/audio/configFrame.cpp
    src/audio/configFrame.h
    src/audio/InputWrapper.cpp
    src/audio/InputWrapper.h
    src/audio/converter/converter.h
    src/audio/converter/converters.cpp
    src/audio/converter/converters.h
    src/audio/converter/converterBackend.cpp
    src/audio/converter/converterBackend.h
    src/audio/converter/converterFactory.cpp
    src/audio/converter/converterFactory.h
    src/audio/converter/quantizer.cpp
    src/audio/converter/quantizer.h
    src/audio/inputTypes.h
    src/audio/input/input.h
    src/audio/input/inputBackend.cpp
    src/audio/input/inputBackend.h
    src/audio/input/inputFactory.cpp
    src/audio/input/inputFactory.h
    src/audio/input/metaData.cpp
    src/audio/input/metaData.h
    src/audio/output/audioProcess.h
    src/audio/output/qaudioBackend.cpp
    src/audio/output/qaudioBackend.h
    src/audio/output/AudioOutputWrapper.h
    src/gui/aboutDialog.cpp
    src/gui/aboutDialog.h
    src/gui/bookmark.cpp
    src/gui/bookmark.h
    src/gui/centralFrame.cpp
    src/gui/centralFrame.h
    src/gui/iconFactory.cpp
    src/gui/iconFactory.h
    src/gui/icons.h
    src/gui/infoDialog.cpp
    src/gui/infoDialog.h
    src/gui/infoLabel.cpp
    src/gui/infoLabel.h
    src/gui/mainWindow.cpp
    src/gui/mainWindow.h
    src/gui/playlist.h
    src/gui/playlistModel.h
    src/gui/proxymodel.h
    src/gui/settings.cpp
    src/gui/settings.h
    src/gui/timeDisplay.h
    src/trackList/trackListBackend.h
    src/trackList/trackListDir.h
    src/trackList/trackListFactory.cpp
    src/trackList/trackListFactory.h
    src/trackList/trackList.h
    src/trackList/trackListM3u.h
    src/trackList/trackListPls.h
    src/trackList/trackListCue.h
    src/utils/AutoDLL.h
    src/utils/AutoDLL.cpp
    src/utils/genres.h
    src/utils/ioEx.h
    src/utils/loadsym.h
    src/utils/utils.cpp
    src/utils/utils.h
    src/utils/tag.cpp
    src/utils/tag.h
    src/utils/xdg.cpp
    src/utils/xdg.h)

if(MINGW)
    list(APPEND MUSIQT_SOURCES
        src/resfile.rc
    )
endif(MINGW)

#####################################################
# Check for debug build

OPTION(DEBUG "Compile for debugging" OFF)
IF(DEBUG)
    ADD_DEFINITIONS(-DDEBUG)
    IF(MINGW)
        target_link_libraries(musiqt -mconsole)
    ENDIF(MINGW)
ELSE(DEBUG)
    ADD_DEFINITIONS(-DNDEBUG)
    ADD_DEFINITIONS(-DQT_NO_DEBUG_OUTPUT)
ENDIF(DEBUG)

#####################################################

include(GNUInstallDirs)
ADD_DEFINITIONS(-DPACKAGE_NAME="${PROJECT_NAME}")
ADD_DEFINITIONS(-DPACKAGE="${PROJECT_NAME}")
ADD_DEFINITIONS(-DPACKAGE_STRING="${PROJECT_NAME} ${CMAKE_PROJECT_VERSION}")
ADD_DEFINITIONS(-DPACKAGE_VERSION="${CMAKE_PROJECT_VERSION}")
ADD_DEFINITIONS(-DPACKAGE_BUGREPORT="${PROJECT_HOMEPAGE_URL}")
ADD_DEFINITIONS(-DLOCALEDIR="${CMAKE_INSTALL_FULL_LOCALEDIR}")

#####################################################
# Check for gettext

find_package(Gettext)
IF(Gettext_FOUND)
    set(LANGUAGE_FILES
        po/it.po
        po/es.po
    )
    GETTEXT_CREATE_TRANSLATIONS(po/musiqt.pot ALL ${LANGUAGE_FILES})
ENDIF(Gettext_FOUND)

#####################################################

add_executable(musiqt ${MUSIQT_SOURCES})

include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/src/audio)
include_directories(${CMAKE_SOURCE_DIR}/src/gui)
include_directories(${CMAKE_SOURCE_DIR}/src/trackList)
include_directories(${CMAKE_SOURCE_DIR}/src/utils)

find_package(PkgConfig REQUIRED)

INSTALL(FILES musiqt.desktop DESTINATION ${CMAKE_INSTALL_PREFIX}/share/applications)
INSTALL(FILES musiqt.png DESTINATION ${CMAKE_INSTALL_PREFIX}/share/icons)

##################################################################
# Check for Qt

find_package(Qt5 COMPONENTS Widgets Network Multimedia REQUIRED)

target_link_libraries(musiqt Qt5::Widgets Qt5::Network Qt5::Multimedia)

##################################################################
# Search for zlib

find_package(ZLIB)
IF(ZLIB_FOUND)
    ADD_DEFINITIONS(-DNOUNCRYPT -DHAVE_LIBZ)
    target_sources(musiqt PRIVATE
        src/libs/unzip/ioapi.h
        src/libs/unzip/ioapi.c
        src/libs/unzip/unzip.h
        src/libs/unzip/unzip.c
    )
    if(MINGW)
        target_sources(musiqt PRIVATE
            src/libs/unzip/iowin32.h
            src/libs/unzip/iowin32.c
        )
    endif(MINGW)
    target_link_libraries(musiqt ZLIB::ZLIB)
ENDIF(ZLIB_FOUND)

##################################################################
# Search for libsndfile

OPTION(SNDFILE "Build sndfile backend if available" ON)
IF(SNDFILE)
    find_package(SndFile 1.0)
    if(SNDFILE_FOUND)
        MESSAGE(STATUS "Found sndfile, version " ${SndFile_VERSION})
        ADD_DEFINITIONS(-DHAVE_SNDFILE)
        target_sources(musiqt PRIVATE
            src/audio/input/sndBackend.cpp
            src/audio/input/sndBackend.h
        )
        target_link_libraries(musiqt SndFile::sndfile)
        if(MINGW AND ${SndFile_VERSION} VERSION_GREATER_EQUAL "1.0.21")
            ADD_DEFINITIONS(-DENABLE_SNDFILE_WINDOWS_PROTOTYPES)
        endif()
    endif(SNDFILE_FOUND)
ENDIF(SNDFILE)

##################################################################
# Search for libmpg123

OPTION(MPG123 "Build mpg123 backend if available" ON)
IF(MPG123)
    pkg_check_modules(libmpg123 IMPORTED_TARGET libmpg123>=1.6)
    if(libmpg123_FOUND)
        ADD_DEFINITIONS(-DHAVE_MPG123)
        target_sources(musiqt PRIVATE
            src/audio/input/mpg123Backend.cpp
            src/audio/input/mpg123Backend.h
        )
        target_link_libraries(musiqt PkgConfig::libmpg123)
    endif(libmpg123_FOUND)
ENDIF(MPG123)

##################################################################
# Search for vorbisfile

OPTION(VORBIS "Build ogg-vorbis backend if available" ON)
IF(VORBIS)
    pkg_check_modules(vorbisfile IMPORTED_TARGET vorbisfile>=1.1)
    if(vorbisfile_FOUND)
        ADD_DEFINITIONS(-DHAVE_VORBIS)
        target_sources(musiqt PRIVATE
            src/audio/input/oggBackend.cpp
            src/audio/input/oggBackend.h
        )
        target_link_libraries(musiqt PkgConfig::vorbisfile)
    endif(vorbisfile_FOUND)
ENDIF(VORBIS)

##################################################################
# Search for opusfile

OPTION(OPUS "Build opus backend if available" ON)
IF(OPUS)
    pkg_check_modules(opusfile IMPORTED_TARGET opusfile>=0.7)
    if(opusfile_FOUND)
        ADD_DEFINITIONS(-DHAVE_OPUS)
        target_sources(musiqt PRIVATE
            src/audio/input/opusBackend.cpp
            src/audio/input/opusBackend.h
        )
        target_link_libraries(musiqt PkgConfig::opusfile)
    endif(opusfile_FOUND)
ENDIF(OPUS)

##################################################################
# Search for Game Music Engine

OPTION(GME "Build Game Music Engine backend if available" ON)
IF(GME)
    pkg_check_modules(libgme IMPORTED_TARGET libgme>=0.6)
    if(libgme_FOUND)
        ADD_DEFINITIONS(-DHAVE_GME)
        target_sources(musiqt PRIVATE
            src/audio/input/gmeBackend.cpp
            src/audio/input/gmeBackend.h
        )
        target_link_libraries(musiqt PkgConfig::libgme)
    endif(libgme_FOUND)
ENDIF(GME)

##################################################################
# Search for libopenmpt

OPTION(OPENMPT "Build openmpt backend if available" ON)
IF(OPENMPT)
    pkg_check_modules(libopenmpt IMPORTED_TARGET libopenmpt>=0.2)
    if(libopenmpt_FOUND)
        ADD_DEFINITIONS(-DHAVE_OPENMPT)
        target_sources(musiqt PRIVATE
            src/audio/input/openmptBackend.cpp
            src/audio/input/openmptBackend.h
        )
        target_link_libraries(musiqt PkgConfig::libopenmpt)
    endif(libopenmpt_FOUND)
ENDIF(OPENMPT)

##################################################################
# Search for wavpack

OPTION(WAVPACK "Build wavpack backend if available" ON)
IF(WAVPACK)
    pkg_check_modules(wavpack IMPORTED_TARGET wavpack>=4)
    if(wavpack_FOUND)
        ADD_DEFINITIONS(-DHAVE_WAVPACK)
        target_sources(musiqt PRIVATE
            src/audio/input/wvBackend.cpp
            src/audio/input/wvBackend.h
        )
        target_link_libraries(musiqt PkgConfig::wavpack)
    endif(wavpack_FOUND)
ENDIF(WAVPACK)

##################################################################
# Search for ffmpeg

OPTION(FFMPEG "Build ffmpeg backend if available" ON)
IF(FFMPEG)
    INCLUDE(CheckIncludeFile)
    CHECK_INCLUDE_FILE(libavformat/avformat.h HAVE_AVFORMAT_H -D__STDC_CONSTANT_MACROS)
    if(HAVE_AVFORMAT_H)
        ADD_DEFINITIONS(-D__STDC_CONSTANT_MACROS)
        target_sources(musiqt PRIVATE
            src/audio/input/ffmpegBackend.cpp
            src/audio/input/ffmpegBackend.h
        )
    endif(HAVE_AVFORMAT_H)
ENDIF(FFMPEG)

##################################################################
# Search for libsidplayfp

OPTION(SIDPLAYFP "Build sidplayfp backend if available" ON)
IF(SIDPLAYFP)
    pkg_check_modules(libsidplayfp IMPORTED_TARGET libsidplayfp>=1.0)
    if(libsidplayfp_FOUND)
        ADD_DEFINITIONS(-DHAVE_SIDPLAYFP)
        target_sources(musiqt PRIVATE
            src/audio/input/sidBackend.cpp
            src/audio/input/sidBackend.h
        )
        target_link_libraries(musiqt PkgConfig::libsidplayfp)

        INCLUDE(CheckIncludeFileCXX)
        LIST(APPEND CMAKE_REQUIRED_INCLUDES ${libsidplayfp_INCLUDE_DIRS})
        CHECK_INCLUDE_FILE_CXX(sidplayfp/builders/residfp.h HAVE_SIDPLAYFP_BUILDERS_RESIDFP_H)
        CHECK_INCLUDE_FILE_CXX(sidplayfp/builders/resid.h HAVE_SIDPLAYFP_BUILDERS_RESID_H)
        CHECK_INCLUDE_FILE_CXX(sidplayfp/builders/hardsid.h HAVE_SIDPLAYFP_BUILDERS_HARDSID_H)
        CHECK_INCLUDE_FILE_CXX(sidplayfp/builders/exsid.h HAVE_SIDPLAYFP_BUILDERS_EXSID_H)
    endif(libsidplayfp_FOUND)
ENDIF(SIDPLAYFP)

##################################################################
# Check if we need STIL support

if(libsidplayfp_FOUND OR libgme_FOUND)
    pkg_check_modules(libstilview IMPORTED_TARGET libstilview>=1.0)
    if(libstilview_FOUND)
        ADD_DEFINITIONS(-DHAVE_STILVIEW)
        target_link_libraries(musiqt PkgConfig::libstilview)
    endif(libstilview_FOUND)
endif()

##################################################################
# Check if we should build Hively backend

option(HVL "Build Hively backend" ON)
if(HVL)
    ADD_DEFINITIONS(-DBUILD_HVL)
    target_sources(musiqt PRIVATE
        src/libs/hvl_replay/hvl_replay.h
        src/libs/hvl_replay/hvl_replay.c
        src/libs/hvl_replay/hvl_tables.h
        src/libs/hvl_replay/hvl_tables.c
        src/audio/input/hvlBackend.h
        src/audio/input/hvlBackend.cpp
    )
endif(HVL)

##################################################################
# Search for libmpc

OPTION(MPC "Build mpc backend if available" ON)
IF(MPC)
    INCLUDE(CheckLibraryExists)
    CHECK_INCLUDE_FILE(mpc/mpcdec.h HAVE_MPC_MPCDEC_H)
    if(HAVE_MPC_MPCDEC_H)
        CHECK_LIBRARY_EXISTS(mpcdec mpc_demux_init "" HAVE_LIBMPCDEC)
    else(HAVE_MPC_MPCDEC_H)
        CHECK_INCLUDE_FILE(mpcdec/mpcdec.h HAVE_MPCDEC_MPCDEC_H)
        if(HAVE_MPCDEC_MPCDEC_H)
            CHECK_LIBRARY_EXISTS(mpcdec mpc_decoder_initialize "" HAVE_LIBMPCDEC)
        endif(HAVE_MPCDEC_MPCDEC_H)
    endif(HAVE_MPC_MPCDEC_H)

    if(HAVE_LIBMPCDEC)
        target_sources(musiqt PRIVATE
            src/audio/input/mpcBackend.cpp
            src/audio/input/mpcBackend.h
        )
    endif(HAVE_LIBMPCDEC)
ENDIF(MPC)

##################################################################
# Search for bs2b

pkg_check_modules(libbs2b IMPORTED_TARGET libbs2b>=2.2)
if(libbs2b_FOUND)
    ADD_DEFINITIONS(-DHAVE_BS2B)
    target_link_libraries(musiqt PkgConfig::libbs2b)
endif(libbs2b_FOUND)
