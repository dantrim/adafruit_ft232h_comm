# - Try to find libmpsse
# Once done this will define
#  LIBPICO_FOUND - System has libmpsse
#  LIBPICO_INCLUDE_DIRS - The libmpsse include directories
#  LIBPICO_LIBRARIES - The libraries needed to use libmpsse
#  LIBPICO_DEFINITIONS - Compiler switches required for using libmpsse

FIND_PATH(LIBPICO_INCLUDE_DIR libps6000-1.4/ps6000Api.h
          HINTS /usr/include /opt/picoscope/include)

FIND_LIBRARY(LIBPICO_LIBRARY NAMES ps6000 libps6000
              HINTS /usr/lib64 /opt/picoscope/lib )

INCLUDE(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set LIBPICO_FOUND to TRUE
# if all listed variables are TRUE
FIND_PACKAGE_HANDLE_STANDARD_ARGS(libpico DEFAULT_MSG
  LIBPICO_LIBRARY LIBPICO_INCLUDE_DIR)

MARK_AS_ADVANCED(LIBPICO_INCLUDE_DIR LIBPICO_LIBRARY )

SET(LIBPICO_LIBRARIES ${LIBPICO_LIBRARY} )
SET(LIBPICO_INCLUDE_DIRS ${LIBPICO_INCLUDE_DIR} )
