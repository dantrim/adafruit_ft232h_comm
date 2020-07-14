cmake_minimum_required (VERSION 3.12)

project (dantrim_ft232h)

##
## VERSION INFO
##
set (dantrim_ft232h_VERSION_MAJOR 0)
set (dantrim_ft232h_VERSION_MINOR 1)

##
## BUILD STRUCTURE
##
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_MACOSX_RPATH 1)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
#set(CMAKE_MODULE_APTH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/labRemote/cmake")

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.8)
    message(FATAL_ERROR "GCC version must be at least 4.8!")
  endif()
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 3.2)
    message(FATAL_ERROR "Clang version must be at least 3.2!")
  endif()
else()
  message(WARNING "You are using an unsupported compiler! Compilation has only been tested with Clang and GCC.")
endif()

if(CMAKE_BUILD_TYPE MATCHES Debug)
  add_compile_options(-g)
else()
  add_compile_options(-O2)
endif()

set(CMAKE_CXX_STANDARD 14)

# Source code
#add_subdirectory(labRemote)
add_subdirectory(src)
