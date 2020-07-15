##
## add up all main files
##
file(GLOB tools [a-zA-Z]*.cpp)

foreach(target ${tools})
    get_filename_component(execname ${target} NAME_WE)
    get_filename_component(srcfile ${target} NAME)

    add_executable(${execname})
    target_sources(${execname} PRIVATE ${srcfile})
    target_link_libraries(${execname} PRIVATE ${LIBFTDI_LIBRARIES} ${LIBMPSSE_LIBRARIES})
endforeach()
