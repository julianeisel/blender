# - Find Wayland-client library
# Find the native Wayland-client includes and library

#=============================================================================
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================

# If WAYLAND_CLIENT_ROOT_DIR was defined in the environment, use it.
IF(NOT WAYLAND_CLIENT_ROOT_DIR AND NOT $ENV{WAYLAND_CLIENT_ROOT_DIR} STREQUAL "")
  SET(WAYLAND_CLIENT_ROOT_DIR $ENV{WAYLAND_CLIENT_ROOT_DIR})
ENDIF()

SET(_wayland_SEARCH_DIRS
  ${WAYLAND_CLIENT_ROOT_DIR}
  /usr/local
  /sw # Fink
  /opt/local # DarwinPorts
)

FIND_PATH(WAYLAND_CLIENT_INCLUDE_DIR
  NAMES
    wayland-client.h
  HINTS
    ${_wayland_SEARCH_DIRS}
  PATH_SUFFIXES
    include
)

FIND_LIBRARY(WAYLAND_CLIENT_LIBRARY
  NAMES
    wayland-client
  HINTS
    ${_wayland_SEARCH_DIRS}
  PATH_SUFFIXES
    lib64 lib
)

# handle the QUIETLY and REQUIRED arguments and set OPTIX_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(wayland-client DEFAULT_MSG
    WAYLAND_CLIENT_LIBRARY WAYLAND_CLIENT_INCLUDE_DIR)

IF(WAYLAND_CLIENT_FOUND)
  SET(WAYLAND_CLIENT_LIBRARIES ${WAYLAND_CLIENT_LIBRARY})
  SET(WAYLAND_CLIENT_INCLUDE_DIRS ${WAYLAND_CLIENT_INCLUDE_DIR})
ENDIF(WAYLAND_CLIENT_FOUND)

MARK_AS_ADVANCED(
  WAYLAND_CLIENT_INCLUDE_DIR
  WAYLAND_CLIENT_LIBRARY
)

UNSET(_wayland_SEARCH_DIRS)
