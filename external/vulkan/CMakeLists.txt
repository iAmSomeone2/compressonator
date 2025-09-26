# -------------------------------------------------------------------------
# Vulkan

if (NOT OPTION_CMP_VULKAN)
    message("OPTION_CMP_VULKAN=OFF")
    return()
endif()

add_library(ExtVulkan INTERFACE)

# Set the required Vulkan SDK components
set(VK_COMPONENTS glslang SPIRV-Tools)
if (CMP_HOST_APPLE)
    # Add MoltenVK to the list if on an Apple system
    list(APPEND VK_COMPONENTS MoltenVK)
endif()

find_package(Vulkan 1.2 REQUIRED COMPONENTS ${VK_COMPONENTS})

# Link Vulkan libs common across all OSes
target_link_libraries(ExtVulkan INTERFACE Vulkan::Vulkan)

# Handle Apple-specific configs
if (CMP_HOST_APPLE)
    target_link_libraries(ExtVulkan INTERFACE Vulkan::MoltenVK)

    find_library(IO_KIT_FRAMEWORK       IOKit)
    find_library(IO_SURFACE_FRAMEWORK   IOSurface)
    find_library(METAL_FRAMEWORK        Metal)
    find_library(QUARTZ_CORE_FRAMEWORK  QuartzCore)

    target_link_libraries(ExtVulkan INTERFACE
        ${IO_KIT_FRAMEWORK}
        ${IO_SURFACE_FRAMEWORK}
        ${METAL_FRAMEWORK}
        ${QUARTZ_CORE_FRAMEWORK}
    )
endif()

target_link_libraries(ExtVulkan INTERFACE ${Vulkan_LIBRARIES})
target_include_directories(ExtVulkan INTERFACE 
    ${Vulkan_INCLUDE_DIRS}
)

add_library(ExtGlslang INTERFACE)
target_link_libraries(ExtGlslang INTERFACE Vulkan::Glslang Vulkan::SPIRV-Tools)