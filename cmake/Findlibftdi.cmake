# - Try to find libftdi1
# Once done this will define
#  LIBFTDI_FOUND - System has libftdi1
#  LIBFTDI_INCLUDE_DIRS - The libftdi1 include directories
#  LIBFTDI_LIBRARIES - The libraries needed to use libftdi1
#  LIBFTDI_DEFINITIONS - Compiler switches required for using libftdi1

FIND_PATH(LIBFTDI_INCLUDE_DIR ftdi.h
  HINTS /usr/include/libftdi1 /usr/local/include/libftdi1 ~/lib/ftdi/include/libftdi1 )

FIND_LIBRARY(LIBFTDI_LIBRARY NAMES ftdi1 libftdi1 libftdi
  HINTS /usr/lib64 /usr/local/lib /usr/lib/x86_64-linux-gnu ~/lib/ftdi/lib )

INCLUDE(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set LIBFTDI_FOUND to TRUE
# if all listed variables are TRUE
FIND_PACKAGE_HANDLE_STANDARD_ARGS(libftdi  DEFAULT_MSG
  LIBFTDI_LIBRARY LIBFTDI_INCLUDE_DIR)

MARK_AS_ADVANCED(LIBFTDI_INCLUDE_DIR LIBFTDI_LIBRARY )

SET(LIBFTDI_LIBRARIES ${LIBFTDI_LIBRARY} )
SET(LIBFTDI_INCLUDE_DIRS ${LIBFTDI_INCLUDE_DIR} )
