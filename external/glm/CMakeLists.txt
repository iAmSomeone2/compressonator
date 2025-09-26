message(STATUS "++++++++++++++++++ External-GLM" )

# -------------------------------------------------------------------------

#add_subdirectory(${PROJECT_SOURCE_DIR}/../common/lib/ext/glm ${PROJECT_SOURCE_DIR}/../common/lib/ext/glm EXCLUDE_FROM_ALL)

# -------------------------------------------------------------------------
# GLM

#add_library(ExtGLM INTERFACE)

FetchContent_Declare(
    glm
    GIT_REPOSITORY "https://github.com/g-truc/glm.git"
    GIT_TAG "0af55ccecd98d4e5a8d1fad7de25ba429d60e863" # v1.0.1
    GIT_SHALLOW true
    FIND_PACKAGE_ARGS NAMES glm VERSION 1.0
)
#FetchContent_MakeAvailable(glm)

#target_link_libraries(ExtGLM INTERFACE
#        glm::glm
#)
#
#target_include_directories(ExtGLEW INTERFACE
#    ${PROJECT_SOURCE_DIR}/../common/lib/ext/glm
#)