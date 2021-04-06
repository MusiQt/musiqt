cmake_minimum_required(VERSION 3.12)
project(musiqt VERSION 0.8.0 HOMEPAGE_URL https://github.com/drfiemost/musiqt)

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# stupid cmake doesn't support CPPFLAGS
# append it to cmake flags so the ffmpeg header can be found
# even if installed in a non standard dir
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} $ENV{CPPFLAGS}")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} $ENV{CPPFLAGS}")

set(MUSIQT_SOURCES
    src/resources.qrc
    src/gettext.h
    src/main.cpp
    src/singleApp.cpp
    src/singleApp.h
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
    src/audio/input/input.cpp
    src/audio/input/input.h
    src/audio/input/inputFactory.cpp
    src/audio/input/inputFactory.h
    src/audio/input/metaData.cpp
    src/audio/input/metaData.h
    src/audio/input/oggTag.h
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
        ${CMAKE_CURRENT_BINARY_DIR}/src/resfile.rc
    )
endif(MINGW)

#####################################################

include(GNUInstallDirs)

set(PACKAGE "${PROJECT_NAME}")
set(PACKAGE_NAME "${PROJECT_NAME}")
set(PACKAGE_TARNAME "${PROJECT_NAME}")
set(PACKAGE_STRING "${PROJECT_NAME} ${CMAKE_PROJECT_VERSION}")
set(PACKAGE_VERSION "${CMAKE_PROJECT_VERSION}")
set(PACKAGE_URL "${PROJECT_HOMEPAGE_URL}")
set(PACKAGE_BUGREPORT "${PROJECT_HOMEPAGE_URL}/issues")
set(top_srcdir "${CMAKE_CURRENT_SOURCE_DIR}")

ADD_DEFINITIONS(-DLOCALEDIR="${CMAKE_INSTALL_FULL_LOCALEDIR}")

#####################################################

add_executable(musiqt ${MUSIQT_SOURCES})

include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/src/audio)
include_directories(${CMAKE_SOURCE_DIR}/src/gui)
include_directories(${CMAKE_SOURCE_DIR}/src/trackList)
include_directories(${CMAKE_SOURCE_DIR}/src/utils)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/src)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

find_package(PkgConfig REQUIRED)

INSTALL(TARGETS musiqt RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
INSTALL(FILES musiqt.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
INSTALL(FILES musiqt.png DESTINATION ${CMAKE_INSTALL_DATADIR}/icons)

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
# Check for gettext

OPTION(NLS "Enable Native Language Support" ON)
IF(NLS)
    find_package(Gettext)
    IF(GETTEXT_FOUND)
        SET(ENABLE_NLS 1)
        file(GLOB PO_FILES ${CMAKE_SOURCE_DIR}/po/*.po)
        GETTEXT_CREATE_TRANSLATIONS(po/musiqt.pot ALL ${PO_FILES})
        find_package(Intl REQUIRED)
        include_directories(${Intl_INCLUDE_DIRS})
        target_link_libraries(musiqt ${Intl_LIBRARIES})
        target_sources(musiqt PRIVATE
            src/translator.cpp
            src/translator.h
        )
    ELSE(GETTEXT_FOUND)
        MESSAGE(STATUS "gettext not found, NLS disabled")
    ENDIF(GETTEXT_FOUND)
ENDIF(NLS)

find_program(XGETTEXT_CMD xgettext)
SET(POT_FILE ${CMAKE_SOURCE_DIR}/po/${PROJECT_NAME}.pot)
file(GLOB_RECURSE SRC_FILES ${PROJECT_SOURCE_DIR}/src/*.cpp)
add_custom_target(update_pot 
    COMMAND ${XGETTEXT_CMD} -C -s -j
        -d ${PROJECT_NAME}
        -o ${POT_FILE}
        --from-code=utf-8
        --keyword=tr
        --package-name=${PROJECT_NAME}
        --package-version=${CMAKE_PROJECT_VERSION}
        ${SRC_FILES}
    DEPENDS ${SRC_FILES}
    COMMENT "Extract translatable messages to ${POT_FILE}"
)

##################################################################
# Check for Qt

find_package(Qt5 COMPONENTS Widgets Network Multimedia  REQUIRED)
target_link_libraries(musiqt Qt5::Widgets Qt5::Network Qt5::Multimedia)

find_package(Qt5 OPTIONAL_COMPONENTS DBus)
if (Qt5DBus_FOUND)
    SET(ENABLE_DBUS 1)
    qt5_add_dbus_adaptor( MPRIS_ADAPTOR_SRC
            org.mpris.MediaPlayer2.Player.xml
            dbusHandler.h
            dbusHandler )
    target_sources(musiqt PRIVATE
        ${MPRIS_ADAPTOR_SRC}
        src/dbusHandler.h
        src/dbusHandler.cpp
    )
    target_link_libraries(musiqt Qt5::DBus)
endif()

##################################################################
# Search for zlib

find_package(ZLIB)
IF(ZLIB_FOUND)
    ADD_DEFINITIONS(-DNOUNCRYPT)
    SET(HAVE_LIBZ 1)
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
    #find_package(SndFile 1.0)
    pkg_check_modules(sndfile IMPORTED_TARGET sndfile>=1.0)
    if(sndfile_FOUND)
        #MESSAGE(STATUS "Found sndfile, version " ${SndFile_VERSION})
        SET(HAVE_SNDFILE 1)
        target_sources(musiqt PRIVATE
            src/audio/input/sndBackend.cpp
            src/audio/input/sndBackend.h
        )
        target_link_libraries(musiqt PkgConfig::sndfile)
        if(MINGW AND ${sndfile_VERSION} VERSION_GREATER_EQUAL "1.0.21")
            SET(ENABLE_SNDFILE_WINDOWS_PROTOTYPES 1)
        endif()
    endif(sndfile_FOUND)
ENDIF(SNDFILE)

##################################################################
# Search for libmpg123

OPTION(MPG123 "Build mpg123 backend if available" ON)
IF(MPG123)
    pkg_check_modules(libmpg123 IMPORTED_TARGET libmpg123>=1.6)
    if(libmpg123_FOUND)
        SET(HAVE_MPG123 1)
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
        SET(HAVE_VORBIS 1)
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
        SET(HAVE_OPUS 1)
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
        SET(HAVE_GME 1)
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
    pkg_check_modules(libopenmpt IMPORTED_TARGET libopenmpt<0.5)
    if(libopenmpt_FOUND)
        SET(HAVE_OPENMPT 1)
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
        SET(HAVE_WAVPACK 1)
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
        SET(HAVE_LIBAVFORMAT_AVFORMAT_H 1)
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
        SET(HAVE_SIDPLAYFP 1)
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
        SET(HAVE_STILVIEW 1)
        target_link_libraries(musiqt PkgConfig::libstilview)
    endif(libstilview_FOUND)
endif()

##################################################################
# Check if we should build Hively backend

option(HVL "Build Hively backend" ON)
if(HVL)
    SET(BUILD_HVL 1)
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
        set(CMAKE_REQUIRED_LIBRARIES -lm) # ???
        CHECK_LIBRARY_EXISTS(mpcdec mpc_demux_init "" HAVE_LIBMPCDEC)
        set(MPCDEC_SV8 1)
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
        target_link_libraries(musiqt mpcdec)
    endif(HAVE_LIBMPCDEC)
ENDIF(MPC)

##################################################################
# Search for bs2b

pkg_check_modules(libbs2b IMPORTED_TARGET libbs2b>=2.2)
if(libbs2b_FOUND)
    SET(HAVE_BS2B 1)
    target_link_libraries(musiqt PkgConfig::libbs2b)
endif(libbs2b_FOUND)

##################################################################

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/src/resfile.rc.in ${CMAKE_CURRENT_BINARY_DIR}/src/resfile.rc @ONLY)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/src/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/src/config.h @ONLY)

ADD_DEFINITIONS(-DHAVE_CONFIG_H)
