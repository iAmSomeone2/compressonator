cmake_minimum_required(VERSION 3.24)

message(STATUS "++++++++++++++++++ External GLFW" )

# Setting default values for GLFW variables so that we build what we need
SET(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)
SET(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Build the GLFW example programs" FORCE)
SET(GLFW_BUILD_TESTS OFF CACHE BOOL "Build the GLFW test programs" FORCE)
SET(GLFW_BUILD_DOCS OFF CACHE BOOL "Build the GLFW documentation" FORCE)
SET(GLFW_INSTALL OFF CACHE BOOL "Generate installation target" FORCE)

FetchContent_Declare(glfw3
    GIT_REPOSITORY "https://github.com/glfw/glfw.git"
    GIT_TAG "7b6aead9fb88b3623e3b3725ebb42670cbe4c579" # v3.4.0
    GIT_SHALLOW true
    FIND_PACKAGE_ARGS 3.4
)
#FetchContent_MakeAvailable(glfw3)
#
#add_library(ExtGLFW INTERFACE)
#target_link_libraries(ExtGLFW INTERFACE glfw)