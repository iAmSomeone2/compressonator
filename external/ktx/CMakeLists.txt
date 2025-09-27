cmake_minimum_required(VERSION 3.24)

# CMakeLists.txt
message(STATUS "++++++++++++++++++ " )
message(STATUS "++++++++++++++++++ External-KTX" )
message(STATUS "++++++++++++++++++ " )

# if(POLICY CMP0091)
#   cmake_policy(SET CMP0091 NEW)
# endif()

FetchContent_Declare(KTX
    GIT_REPOSITORY "https://github.com/KhronosGroup/KTX-Software.git"
    GIT_TAG "beef80159525d9fb7abb8645ea85f4c4f6842e8f" # v4.4
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)
