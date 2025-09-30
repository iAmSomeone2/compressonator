cmake_minimum_required(VERSION 3.24)

include(FetchContent)

set(FOLDER_NAME ${PROJECT_FOLDER_SDK_EXTERNAL})

message(STATUS "++++++++++++++++++ External CMakeList called" )

set(FETCH_PKGS)
if (OPTION_BUILD_APPS_CMP_CLI OR OPTION_BUILD_APPS_CMP_GUI)
    include(${CMAKE_CURRENT_LIST_DIR}/glew/CMakeLists.txt)
    include(${CMAKE_CURRENT_LIST_DIR}/glm/CMakeLists.txt)
    include(${CMAKE_CURRENT_LIST_DIR}/glfw/CMakeLists.txt)
    list(APPEND FETCH_PKGS
            GLEW
            glm
            glfw3
    )

    include(${CMAKE_CURRENT_LIST_DIR}/opengl/CMakeLists.txt)
    include(${CMAKE_CURRENT_LIST_DIR}/qt5/CMakeLists.txt)
    include_directories(${CMAKE_CURRENT_LIST_DIR}/rapidxml ${CMAKE_CURRENT_LIST_DIR}/include)
    include(${CMAKE_CURRENT_LIST_DIR}/vulkan/CMakeLists.txt)
endif()

if (OPTION_BUILD_BROTLIG)
    include(${CMAKE_CURRENT_LIST_DIR}/brotlig/CMakeLists.txt)
    list(APPEND FETCH_PKGS brotlig)
endif()

# enable KTX2 features
if(OPTION_BUILD_KTX2)
    include(${CMAKE_CURRENT_LIST_DIR}/ktx/CMakeLists.txt)
    list(APPEND FETCH_PKGS KTX)
endif()

list(APPEND CMAKE_FIND_ROOT_PATH ${DEPENDENCIES_INSTALL_DIR})

#append know paths to cmake search path
#list(APPEND CMAKE_FIND_ROOT_PATH ${CMAKE_CURRENT_LIST_DIR}/glew/glew_1.9.0)

set(PATH_SUFFIXES lib)

# ====================================================
#                    COMMON
# ====================================================
find_package(Threads REQUIRED QUIET)

include(${CMAKE_CURRENT_LIST_DIR}/zlib/CMakeLists.txt)
list(APPEND FETCH_PKGS ZLIB)

# ====================================================
#                    ImGui
# ====================================================

if (OPTION_BUILD_APPS_CMP_GUI AND NOT CMP_HOST_APPLE)
    add_library(ImGui STATIC)

    # Add standard required sources from root
    file(GLOB imgui_base_srcs
        "${CMAKE_CURRENT_LIST_DIR}/imgui/*.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/imgui/*.h"
        "${CMAKE_CURRENT_LIST_DIR}/imgui/backends/imgui_impl_vulkan.h"
        "${CMAKE_CURRENT_LIST_DIR}/imgui/backends/imgui_impl_vulkan.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/imgui/backends/imgui_impl_glfw.h"
        "${CMAKE_CURRENT_LIST_DIR}/imgui/backends/imgui_impl_glfw.cpp"
    )

    # Link Vulkan and GLFW
    target_link_libraries(ImGui PRIVATE Vulkan::Vulkan glfw)

    # Add Windows sources
    set(imgui_win_srcs)
    if (CMP_HOST_WINDOWS)
        list(APPEND imgui_win_srcs
                "${CMAKE_CURRENT_LIST_DIR}/imgui/backends/imgui_impl_dx11.h"
                "${CMAKE_CURRENT_LIST_DIR}/imgui/backends/imgui_impl_dx11.cpp"
                "${CMAKE_CURRENT_LIST_DIR}/imgui/backends/imgui_impl_dx12.h"
                "${CMAKE_CURRENT_LIST_DIR}/imgui/backends/imgui_impl_dx12.cpp"
                "${CMAKE_CURRENT_LIST_DIR}/imgui/backends/imgui_impl_win32.h"
                "${CMAKE_CURRENT_LIST_DIR}/imgui/backends/imgui_impl_win32.cpp"
        )
    endif()

    target_sources(ImGui PUBLIC 
        ${imgui_base_srcs}
        ${imgui_win_srcs}
    )

    # Add backends folder to include paths
    target_include_directories(ImGui PUBLIC 
        "${CMAKE_CURRENT_LIST_DIR}/imgui/backends"
        "${CMAKE_CURRENT_LIST_DIR}/imgui"
    )
endif()

# ====================================================
#                  DIRECTX
# ====================================================
if (OPTION_CMP_DIRECTX)
    message(STATUS "++++++++++++++++++ DIRECTX 12")
    add_library(ExtDirectX INTERFACE)

    target_link_libraries(ExtDirectX INTERFACE
        d3d12.lib
        dxgi.lib
        d3dcompiler.lib
    )
endif()

# ====================================================
#                DIRECTX TEX
# ====================================================
if (OPTION_CMP_DIRECTX)
    message(STATUS "++++++++++++++++++ DIRECTX TEX")
    include(${CMAKE_CURRENT_LIST_DIR}/directxtex/CMakeLists.txt)
    # add_library(ExtDirectXTex INTERFACE)
    target_link_libraries(ExtDirectXTex INTERFACE
        DirectXTex
    )
endif()

# ====================================================
#                     OPENCV
# ====================================================
if (OPTION_CMP_OPENCV)
    list(APPEND CMAKE_FIND_ROOT_PATH ${CMAKE_CURRENT_LIST_DIR}/opencv/)
    message(STATUS "++++++++++++++++++  OPENCV" )
    include(${CMAKE_CURRENT_LIST_DIR}/opencv/CMakeLists.txt)
endif()

# ====================================================
#                   OPENEXR
# ====================================================
if (OPTION_BUILD_EXR)
    message(STATUS "++++++++++++++++++ O P E N E X R" )
    include(${CMAKE_CURRENT_LIST_DIR}/openexr/CMakeLists.txt)
    list(APPEND FETCH_PKGS OpenEXR)
endif()

FetchContent_MakeAvailable("${FETCH_PKGS}")
