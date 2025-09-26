cmake_minimum_required(VERSION 3.24)

message(STATUS "++++++++++++++++++ External-GLEW" )

FetchContent_Declare(
        GLEW
        GIT_REPOSITORY "https://github.com/nigels-com/glew.git"
        GIT_TAG "9fb23c3e61cbd2d581e33ff7d8579b572b38ee26" # v2.2.0
        GIT_SHALLOW true
        SOURCE_DIR build/cmake
        FIND_PACKAGE_ARGS
)
