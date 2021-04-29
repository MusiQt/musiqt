include(GNUInstallDirs)

find_path(LASTFM_INCLUDE_DIR
  NAMES lastfm5/ws.h
  HINTS ENV LASTFM_DIR
  PATH_SUFFIXES include)

find_library(
    LASTFM_LIBRARY
    NAMES lastfm5 liblastfm5
    HINTS ENV LASTFM_DIR
    PATH_SUFFIXES lib)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(LASTFM DEFAULT_MSG
                                  LASTFM_LIBRARY
                                  LASTFM_INCLUDE_DIR)

if(LASTFM_FOUND)
  set(LASTFM_INCLUDE_DIRS ${LASTFM_INCLUDE_DIR})
  set(LASTFM_LIBRARIES ${LASTFM_LIBRARY})

  if(NOT TARGET LASTFM::LASTFM)
    add_library(LASTFM::LASTFM UNKNOWN IMPORTED)
    set_target_properties(LASTFM::LASTFM PROPERTIES
      IMPORTED_LOCATION ${LASTFM_LIBRARY}
      INTERFACE_INCLUDE_DIRECTORIES ${LASTFM_INCLUDE_DIRS})
  endif()
endif(LASTFM_FOUND)

mark_as_advanced(LASTFM_INCLUDE_DIR LASTFM_LIBRARY)
