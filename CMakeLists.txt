cmake_minimum_required(VERSION 3.24.0)
project(ergvok LANGUAGES CXX)

set(ERGOVK_BUILD_EXAMPLE "TRUE" CACHE BOOL "Build the example program")
set(ERGOVK_VULKAN_SDK "$ENV{VULKAN_SDK}" CACHE PATH "Path to the Vulkan SDK root")

set(Vulkan_FIND_COMPONENTS "shaderc_combined")
include(FindVulkan)
if(NOT Vulkan_FOUND)
message(FATAL_ERROR "Cannot find find Vulkan SDK")
endif()
if(NOT IS_DIRECTORY ${ERGOVK_VULKAN_SDK})
message(FATAL_ERROR "VULKAN_SDK env not present - did you run setup-env.sh?")
endif()

add_subdirectory(src/lib)
if(ERGOVK_BUILD_EXAMPLE)
add_subdirectory(src/example)
endif()