# - Try to find libgclib
# Once done this will define
#  LIBGCLIB_FOUND - System has libgclib
#  LIBGCLIB_INCLUDE_DIRS - The libgclib include directories
#  LIBGCLIB_LIBRARIES - The libraries needed to use libgclib
#  LIBGCLIB_DEFINITIONS - Compiler switches required for using libgclib

FIND_PATH(LIBGCLIB_INCLUDE_DIR gclib.h
          HINTS /usr/include)

FIND_LIBRARY(LIBGCLIB_LIBRARY NAMES libgclib libgclib.so.0 libgclib.so libgclib.so.0.422
              HINTS /usr/lib )

INCLUDE(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set gclib_FOUND to TRUE
# if all listed variables are TRUE
FIND_PACKAGE_HANDLE_STANDARD_ARGS(libgclib DEFAULT_MSG
  LIBGCLIB_LIBRARY LIBGCLIB_INCLUDE_DIR)

MARK_AS_ADVANCED(LIBGCLIB_INCLUDE_DIR LIBGCLIB_LIBRARY )

SET(LIBGCLIB_LIBRARIES ${LIBGCLIB_LIBRARY} )
SET(LIBGCLIB_INCLUDE_DIRS ${LIBGCLIB_INCLUDE_DIR} )
