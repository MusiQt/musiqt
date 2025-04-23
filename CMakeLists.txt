cmake_minimum_required(VERSION 3.12)
project(musiqt VERSION 3.2.0 HOMEPAGE_URL https://github.com/MusiQt/musiqt)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR
        "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    add_compile_options("$<$<CONFIG:DEBUG>:-Wall;-Wextra;-pedantic>")
endif()

# stupid cmake doesn't support CPPFLAGS
# append it to cmake flags so the ffmpeg header can be found
# even if installed in a non standard dir
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} $ENV{CPPFLAGS}")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} $ENV{CPPFLAGS}")

if(WIN32)
    OPTION(PORTABLE "Enable Portable App" OFF)
    if(PORTABLE)
        add_compile_definitions(MUSIQT_PORTABLE_APP)
    endif()
endif()

#####################################################

add_executable(musiqt)

target_include_directories(musiqt PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_include_directories(musiqt PRIVATE ${CMAKE_SOURCE_DIR}/src/audio)
target_include_directories(musiqt PRIVATE ${CMAKE_SOURCE_DIR}/src/gui)
target_include_directories(musiqt PRIVATE ${CMAKE_SOURCE_DIR}/src/trackList)
target_include_directories(musiqt PRIVATE ${CMAKE_SOURCE_DIR}/src/utils)
target_include_directories(musiqt PRIVATE ${CMAKE_BINARY_DIR}/src)
target_include_directories(musiqt PRIVATE ${CMAKE_BINARY_DIR})

target_sources(musiqt PRIVATE
    src/resources.qrc
    src/main.cpp
    src/exceptions.cpp
    src/exceptions.h
    src/singleApp.cpp
    src/singleApp.h
    src/audio/audio.cpp
    src/audio/audio.h
    src/audio/audioState.h
    src/audio/configFrame.cpp
    src/audio/configFrame.h
    src/audio/InputWrapper.cpp
    src/audio/InputWrapper.h
    src/audio/inputConfig.cpp
    src/audio/inputConfig.h
    src/audio/inputTypes.h
    src/audio/inputFactory.cpp
    src/audio/inputFactory.h
    src/audio/loader.cpp
    src/audio/loader.h
    src/audio/metaData.h
    src/audio/player.cpp
    src/audio/player.h
    src/audio/converter/converter.h
    src/audio/converter/converters.cpp
    src/audio/converter/converters.h
    src/audio/converter/converterBackend.cpp
    src/audio/converter/converterBackend.h
    src/audio/converter/converterFactory.cpp
    src/audio/converter/converterFactory.h
    src/audio/converter/quantizer.cpp
    src/audio/converter/quantizer.h
    src/audio/input/input.cpp
    src/audio/input/input.h
    src/audio/input/metaDataImpl.cpp
    src/audio/input/metaDataImpl.h
    src/audio/input/oggTag.cpp
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
    src/utils/loadsym.h
    src/utils/utils.cpp
    src/utils/utils.h
    src/utils/syspaths.cpp
    src/utils/tag.cpp
    src/utils/tag.h
    src/utils/xdg.cpp
    src/utils/xdg.h)

if(WIN32)
    target_sources(musiqt PRIVATE
        ${CMAKE_BINARY_DIR}/src/resfile.rc
    )
endif()

#####################################################

include(GNUInstallDirs)

set(PACKAGE "${PROJECT_NAME}")
set(PACKAGE_NAME "${PROJECT_NAME}")
set(PACKAGE_TARNAME "${PROJECT_NAME}")
set(PACKAGE_STRING "${PROJECT_NAME} ${CMAKE_PROJECT_VERSION}")
set(PACKAGE_VERSION "${CMAKE_PROJECT_VERSION}")
set(PACKAGE_URL "${PROJECT_HOMEPAGE_URL}")
set(PACKAGE_BUGREPORT "${PROJECT_HOMEPAGE_URL}/issues")
set(top_srcdir "${CMAKE_SOURCE_DIR}")

add_compile_definitions(LOCALEDIR="${CMAKE_INSTALL_FULL_LOCALEDIR}")

#####################################################

find_package(PkgConfig REQUIRED)

INSTALL(TARGETS musiqt RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
INSTALL(FILES musiqt.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
INSTALL(FILES musiqt.png DESTINATION ${CMAKE_INSTALL_DATADIR}/icons)

#####################################################
# Check for debug build

add_compile_definitions("$<$<CONFIG:RELEASE>:QT_NO_DEBUG_OUTPUT>")
if(MINGW)
    target_link_libraries(musiqt debug -mconsole optimized -mwindows)
endif()

##################################################################
# Check for LTO

OPTION(LTO "Enable link time optimization" ON)
if(LTO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT have_lto OUTPUT output LANGUAGES CXX)

    if (have_lto)
        message(STATUS "LTO enabled")
        set_property(TARGET musiqt
             PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    else()
        message(WARNING "LTO is not supported: ${output}")
    endif()
endif()

#####################################################
# Check for gettext

OPTION(NLS "Enable Native Language Support" ON)
if(NLS)
    find_package(Gettext)
    if(GETTEXT_FOUND)
        SET(ENABLE_NLS 1)
        file(GLOB PO_FILES ${CMAKE_SOURCE_DIR}/po/*.po)
        GETTEXT_CREATE_TRANSLATIONS(po/musiqt.pot ALL ${PO_FILES})
        find_package(Intl REQUIRED)
        target_include_directories(musiqt PRIVATE ${Intl_INCLUDE_DIRS})
        target_link_libraries(musiqt ${Intl_LIBRARIES})
        target_sources(musiqt PRIVATE
            src/gettext.h
            src/translator.cpp
            src/translator.h
        )
    else()
        MESSAGE(STATUS "gettext not found, NLS disabled")
    endif()
endif()

add_custom_target(
    update_pot
    COMMENT "Updating translated messages"
)

# update the po file
find_program(XGETTEXT_CMD xgettext)
IF (XGETTEXT_CMD)
    SET(POT_FILE ${CMAKE_SOURCE_DIR}/po/${PROJECT_NAME}.pot)
    file(GLOB_RECURSE SRC_FILES ${PROJECT_SOURCE_DIR}/src/*.cpp)
    add_custom_command(
        TARGET update_pot
        PRE_BUILD
        COMMAND ${XGETTEXT_CMD} -C -j
            -d ${PROJECT_NAME}
            -o ${POT_FILE}
            --from-code=utf-8
            --keyword=tr
            --package-name=${PROJECT_NAME}
            --package-version=${CMAKE_PROJECT_VERSION}
            ${SRC_FILES}
        COMMENT "Extracting translatable messages to ${POT_FILE}"
    )
endif()

# update the pot files
if(GETTEXT_MSGMERGE_EXECUTABLE)
    file(GLOB_RECURSE PO_FILES ${PROJECT_SOURCE_DIR}/po/*.po)
    foreach(PO_FILE IN ITEMS ${PO_FILES})
        add_custom_command(
            TARGET update_pot
            POST_BUILD
            COMMAND ${GETTEXT_MSGMERGE_EXECUTABLE} -q -U ${PO_FILE}
                ${CMAKE_SOURCE_DIR}/po/${PROJECT_NAME}.pot
            COMMENT "Updating translation ${PO_FILE}"
        )
    endforeach()
endif()

##################################################################
# Check for Qt

OPTION(QT6 "Enable building with Qt6 if found" ON)
if(QT6)
    find_package(Qt6 COMPONENTS Widgets Network Multimedia)
endif()
if (NOT Qt6_FOUND)
    find_package(Qt5 5.15 COMPONENTS Widgets Network Multimedia REQUIRED)
endif()

target_link_libraries(musiqt Qt::Widgets Qt::Network Qt::Multimedia)

if (UNIX)
    if (Qt6_FOUND)
        find_package(Qt6 OPTIONAL_COMPONENTS DBus)
    else()
        find_package(Qt5 OPTIONAL_COMPONENTS DBus)
    endif()
    if (Qt6DBus_FOUND OR Qt5DBus_FOUND)
        SET(ENABLE_DBUS 1)
        qt_add_dbus_adaptor(MPRIS_ADAPTOR_SRC
                src/org.mpris.MediaPlayer2.xml
                dbusHandler.h
                dbusHandler)
        qt_add_dbus_adaptor(MPRIS_ADAPTOR_SRC
                src/org.mpris.MediaPlayer2.Player.xml
                dbusHandler.h
                dbusHandler)
        target_sources(musiqt PRIVATE
            ${MPRIS_ADAPTOR_SRC}
            src/utils/dbusHandler.h
            src/utils/dbusHandler.cpp
        )
        target_link_libraries(musiqt Qt::DBus)
    endif()
endif()

##################################################################
# Search for zlib

pkg_check_modules(ZLIB IMPORTED_TARGET zlib>=1.0)
if(ZLIB_FOUND)
    add_compile_definitions(NOUNCRYPT)
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
        set_source_files_properties(src/libs/unzip/iowin32.c PROPERTIES COMPILE_FLAGS "-Wno-unused-variable -Wno-unused-parameter")
    endif()
    target_link_libraries(musiqt PkgConfig::ZLIB)
endif()

##################################################################
# Search for libsndfile

OPTION(SNDFILE "Build sndfile backend if available" ON)
if(SNDFILE)
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
    endif()
endif()

##################################################################
# Search for libmpg123

OPTION(MPG123 "Build mpg123 backend if available" ON)
if(MPG123)
    pkg_check_modules(libmpg123 IMPORTED_TARGET libmpg123>=1.6)
    if(libmpg123_FOUND)
        SET(HAVE_MPG123 1)
        target_sources(musiqt PRIVATE
            src/audio/input/mpg123Backend.cpp
            src/audio/input/mpg123Backend.h
        )
        target_link_libraries(musiqt PkgConfig::libmpg123)
    endif()
endif()

##################################################################
# Search for vorbisfile

OPTION(VORBIS "Build ogg-vorbis backend if available" ON)
if(VORBIS)
    pkg_check_modules(vorbisfile IMPORTED_TARGET vorbisfile>=1.1)
    if(vorbisfile_FOUND)
        SET(HAVE_VORBIS 1)
        target_sources(musiqt PRIVATE
            src/audio/input/oggBackend.cpp
            src/audio/input/oggBackend.h
        )
        target_link_libraries(musiqt PkgConfig::vorbisfile)
    endif(vorbisfile_FOUND)
endif(VORBIS)

##################################################################
# Search for opusfile

OPTION(OPUS "Build opus backend if available" ON)
if(OPUS)
    pkg_check_modules(opusfile IMPORTED_TARGET opusfile>=0.7)
    if(opusfile_FOUND)
        SET(HAVE_OPUS 1)
        target_sources(musiqt PRIVATE
            src/audio/input/opusBackend.cpp
            src/audio/input/opusBackend.h
        )
        target_link_libraries(musiqt PkgConfig::opusfile)
    endif()
endif()

##################################################################
# Search for Game Music Engine

OPTION(GME "Build Game Music Engine backend if available" ON)
if(GME)
    pkg_check_modules(libgme IMPORTED_TARGET libgme>=0.6)
    if(libgme_FOUND)
        SET(HAVE_GME 1)
        target_sources(musiqt PRIVATE
            src/audio/input/gmeBackend.cpp
            src/audio/input/gmeBackend.h
        )
        target_link_libraries(musiqt PkgConfig::libgme)
    endif()
endif()

##################################################################
# Search for libopenmpt

OPTION(OPENMPT "Build openmpt backend if available" ON)
if(OPENMPT)
    pkg_check_modules(libopenmpt IMPORTED_TARGET libopenmpt>=0.3)
    if(libopenmpt_FOUND)
        SET(HAVE_OPENMPT 1)
        target_sources(musiqt PRIVATE
            src/audio/input/openmptBackend.cpp
            src/audio/input/openmptBackend.h
        )
        target_link_libraries(musiqt PkgConfig::libopenmpt)
    endif()
endif()

##################################################################
# Search for wavpack

OPTION(WAVPACK "Build wavpack backend if available" ON)
if(WAVPACK)
    pkg_check_modules(wavpack IMPORTED_TARGET wavpack>=4)
    if(wavpack_FOUND)
        SET(HAVE_WAVPACK 1)
        string(REGEX REPLACE "\\..*$" "" WP_MAJOR_VERSION ${wavpack_VERSION})
        SET(WAVPACK_VERSION ${WP_MAJOR_VERSION})
        target_sources(musiqt PRIVATE
            src/audio/input/wvBackend.cpp
            src/audio/input/wvBackend.h
        )
        target_link_libraries(musiqt PkgConfig::wavpack)
    endif()
endif()

##################################################################
# Search for ffmpeg

OPTION(FFMPEG "Build ffmpeg backend if available" ON)
if(FFMPEG)
    INCLUDE(CheckIncludeFile)
    CHECK_INCLUDE_FILE(libavcodec/avcodec.h HAVE_AVCODEC_H)
    CHECK_INCLUDE_FILE(libavformat/avformat.h HAVE_AVFORMAT_H -D__STDC_CONSTANT_MACROS)
    if(HAVE_AVCODEC_H AND HAVE_AVFORMAT_H)
        SET(HAVE_FFMPEG 1)
        add_compile_definitions(__STDC_CONSTANT_MACROS)
        target_sources(musiqt PRIVATE
            src/audio/input/ffmpegBackend.cpp
            src/audio/input/ffmpegBackend.h
        )
    endif()
endif()

##################################################################
# Search for libsidplayfp

OPTION(SIDPLAYFP "Build sidplayfp backend if available" ON)
if(SIDPLAYFP)
    pkg_check_modules(libsidplayfp IMPORTED_TARGET libsidplayfp>=1.0)
    if(libsidplayfp_FOUND)
        SET(HAVE_SIDPLAYFP 1)
        target_sources(musiqt PRIVATE
            src/audio/input/sidBackend.cpp
            src/audio/input/sidBackend.h
            src/audio/input/sidlib_features.h
        )
        target_link_libraries(musiqt PkgConfig::libsidplayfp)

        INCLUDE(CheckIncludeFileCXX)
        LIST(APPEND CMAKE_REQUIRED_INCLUDES ${libsidplayfp_INCLUDE_DIRS})
        CHECK_INCLUDE_FILE_CXX(sidplayfp/builders/residfp.h HAVE_SIDPLAYFP_BUILDERS_RESIDFP_H)
        CHECK_INCLUDE_FILE_CXX(sidplayfp/builders/resid.h HAVE_SIDPLAYFP_BUILDERS_RESID_H)
        CHECK_INCLUDE_FILE_CXX(sidplayfp/builders/hardsid.h HAVE_SIDPLAYFP_BUILDERS_HARDSID_H)
        CHECK_INCLUDE_FILE_CXX(sidplayfp/builders/exsid.h HAVE_SIDPLAYFP_BUILDERS_EXSID_H)
    endif()
endif()

##################################################################
# Check if we need STIL support

if(libsidplayfp_FOUND OR libgme_FOUND)
    pkg_check_modules(libstilview IMPORTED_TARGET libstilview>=1.0)
    if(libstilview_FOUND)
        SET(HAVE_STILVIEW 1)
        target_link_libraries(musiqt PkgConfig::libstilview)
    endif()
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
    set_source_files_properties(src/libs/hvl_replay/hvl_replay.c PROPERTIES COMPILE_FLAGS "-Wno-unused-parameter")
endif()

##################################################################
# Search for libmpc

OPTION(MPC "Build mpc backend if available" ON)
if(MPC)
    INCLUDE(CheckLibraryExists)
    CHECK_INCLUDE_FILE(mpc/mpcdec.h HAVE_MPC_MPCDEC_H)
    if(HAVE_MPC_MPCDEC_H)
        set(CMAKE_REQUIRED_LIBRARIES -lm) # ???
        CHECK_LIBRARY_EXISTS(mpcdec mpc_demux_init "" HAVE_LIBMPCDEC)
        set(MPCDEC_SV8 1)
    else()
        CHECK_INCLUDE_FILE(mpcdec/mpcdec.h HAVE_MPCDEC_MPCDEC_H)
        if(HAVE_MPCDEC_MPCDEC_H)
            CHECK_LIBRARY_EXISTS(mpcdec mpc_decoder_initialize "" HAVE_LIBMPCDEC)
        endif()
    endif()

    if(HAVE_LIBMPCDEC)
        target_sources(musiqt PRIVATE
            src/audio/input/mpcBackend.cpp
            src/audio/input/mpcBackend.h
        )
        target_link_libraries(musiqt mpcdec)
    endif()
endif()

##################################################################
# Search for bs2b

pkg_check_modules(libbs2b IMPORTED_TARGET libbs2b>=3.0)
if(libbs2b_FOUND)
    SET(HAVE_BS2B 1)
    target_link_libraries(musiqt PkgConfig::libbs2b)
endif(libbs2b_FOUND)

##################################################################
# Check for liblastfm

OPTION(LASTFM "Build lastfm support" ON)
if(LASTFM)
    if (Qt6_FOUND)
        find_package(Qt6 OPTIONAL_COMPONENTS Network Xml)
        if(Qt6Xml_FOUND AND Qt6Network_FOUND)
            set(INTERNAL_LIBLASTFM 1)
        endif()
    else()
        find_package(LASTFM)
        if(LASTFM_FOUND)
            find_package(Qt5 OPTIONAL_COMPONENTS Xml)
            if(Qt5Xml_FOUND)
                SET(HAVE_LASTFM 1)
                target_sources(musiqt PRIVATE
                    src/utils/lastfm.cpp
                )
                target_link_libraries(musiqt LASTFM::LASTFM Qt::Xml)
            endif()
        else()
            find_package(Qt5 OPTIONAL_COMPONENTS Network Xml)
            if(Qt5Xml_FOUND AND Qt5Network_FOUND)
                set(INTERNAL_LIBLASTFM 1)
            endif()
        endif()
    endif()

    if (INTERNAL_LIBLASTFM)
        SET(HAVE_LASTFM 1)
        target_include_directories(musiqt PRIVATE ${CMAKE_SOURCE_DIR}/src/libs/liblastfm)

        target_sources(musiqt PRIVATE
            src/libs/liblastfm/ws.cpp
            src/libs/liblastfm/NetworkConnectionMonitor.cpp
            src/libs/liblastfm/NetworkAccessManager.cpp
            src/libs/liblastfm/InternetConnectionMonitor.cpp
            src/libs/liblastfm/User.cpp
            src/libs/liblastfm/Track.cpp
            src/libs/liblastfm/Tag.cpp
            src/libs/liblastfm/Mbid.cpp
            src/libs/liblastfm/Artist.cpp
            src/libs/liblastfm/Album.cpp
            src/libs/liblastfm/ScrobbleCache.cpp
            src/libs/liblastfm/ScrobblePoint.cpp
            src/libs/liblastfm/Audioscrobbler.cpp
            src/libs/liblastfm/XmlQuery.cpp
            src/libs/liblastfm/Url.cpp
            src/libs/liblastfm/UrlBuilder.cpp
            src/libs/liblastfm/misc.cpp
            src/libs/liblastfm/Auth.cpp
            src/libs/liblastfm/Library.cpp
            src/utils/lastfm.cpp
        )

        if(WIN32 AND NOT MINGW)
            add_definitions("-D_ATL_DLL -D_CRT_SECURE_NO_WARNINGS")

            target_sources(musiqt PRIVATE
                src/libs/liblastfm/win/WNetworkConnectionMonitor_win.cpp
                src/libs/liblastfm/win/WmiSink.cpp
                src/libs/liblastfm/win/NdisEvents.cpp
            )
        endif()

        if(APPLE)
            target_sources(musiqt PRIVATE
                src/libs/liblastfm/mac/MNetworkConnectionMonitor_mac.cpp
            )

            find_library(SYSTEMCONFIGURATION_LIBRARY SystemConfiguration)
            find_library(COREFOUNDATION_LIBRARY CoreFoundation)
            target_link_libraries(musiqt
                ${COREFOUNDATION_LIBRARY}
                ${SYSTEMCONFIGURATION_LIBRARY}
            )
        endif()

        if(UNIX AND NOT APPLE)
            target_sources(musiqt PRIVATE
                src/libs/liblastfm/linux/LNetworkConnectionMonitor_linux.cpp
            )
        endif()

        target_link_libraries(musiqt Qt::Network Qt::Xml)
    endif()
endif()

##################################################################

CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/src/resfile.rc.in ${CMAKE_BINARY_DIR}/src/resfile.rc @ONLY)
CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/src/config.h.in ${CMAKE_BINARY_DIR}/src/config.h @ONLY)

add_compile_definitions(HAVE_CONFIG_H)
