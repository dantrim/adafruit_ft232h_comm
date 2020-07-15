##
## DEPENDENCIES
##
find_package( libftdi )
find_package( libmpsse )

if(${LIBFTDI_FOUND})
    add_definitions(-DLIBFTDI1)
endif()

##
## main
##
add_subdirectory(tools)
