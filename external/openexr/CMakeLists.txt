cmake_minimum_required(VERSION 3.24)

# CMakeLists.txt
message(STATUS "++++++++++++++++++ External Adding OpenEXR include paths")

FetchContent_Declare(OpenEXR
    GIT_REPOSITORY https://github.com/AcademySoftwareFoundation/openexr.git
    GIT_TAG 20a65852895894434bea88613f6d29ac8e88bd6e # v3.4.0
    GIT_PROGRESS ON
    GIT_SHALLOW ON
    FIND_PACKAGE_ARGS
)

#add_library(ExtOpenEXR INTERFACE)
#
#if (CMP_HOST_WINDOWS)
#    target_include_directories(ExtOpenEXR INTERFACE
#        ${CMAKE_CURRENT_LIST_DIR}/../../../common/lib/ext/openexr/openexr-2.2.0/VS2015/x64/include/OpenEXR
#        ${CMAKE_CURRENT_LIST_DIR}/../../../common/lib/ext/openexr/ilmbase-2.2.0/VS2015/x64/include/OpenEXR
#    )
#
#    set(OpenEXR_INCLUDE_DIRS
#        ${CMAKE_CURRENT_LIST_DIR}/../../../common/lib/ext/openexr/openexr-2.2.0/VS2015/x64/include/OpenEXR
#        ${CMAKE_CURRENT_LIST_DIR}/../../../common/lib/ext/openexr/ilmbase-2.2.0/VS2015/x64/include/OpenEXR
#        )
#else()
#    set(OpenEXR_INCLUDE_DIRS
#        /usr/include/Imath
#        /usr/include/OpenEXR
#        )
#    target_include_directories(ExtOpenEXR INTERFACE ${OpenEXR_INCLUDE_DIRS})
#endif()

