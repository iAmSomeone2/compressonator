cmake_minimum_required(VERSION 3.24)

#if(POLICY CMP0091)
#  cmake_policy(SET CMP0091 NEW)
#endif()
#
#project(CompressonatorZlib)

set(ZLIB_COMPAT ON)
set(ZLIB_ENABLE_TESTS OFF)
set(WITH_GTEST OFF)
set(ZLIB_USE_STATIC_LIBS ON)

FetchContent_Declare(ZLIB
    GIT_REPOSITORY https://github.com/zlib-ng/zlib-ng.git
    GIT_TAG 425439062b114a0f6cf625022c41d929c7e879f9 #v2.2.5 release
    GIT_PROGRESS ON
    GIT_SHALLOW ON
    FIND_PACKAGE_ARGS
)

