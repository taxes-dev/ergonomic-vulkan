if(NOT IS_DIRECTORY "${ERGOVK_VULKAN_SDK}/include/vma")
message(FATAL_ERROR "Cannot find VMA headers - did you install as part of Vulkan SDK?")
endif()

add_library(ergovk STATIC
    ErrorToString.cpp
    VmaImpl.cpp
    VulkanInstance.cpp
    VulkanInstanceBuilder.cpp
    VkBootstrap.cpp)
target_compile_features(ergovk PUBLIC cxx_std_20)
target_include_directories(ergovk PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(ergovk PRIVATE Vulkan::Vulkan Vulkan::shaderc_combined)
target_compile_options(ergovk PRIVATE -Wno-nullability-completeness)
