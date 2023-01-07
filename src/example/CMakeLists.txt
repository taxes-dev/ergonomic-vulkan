find_package(SDL2 CONFIG REQUIRED)

add_executable(example main.cpp)
target_compile_features(example PUBLIC cxx_std_20)
add_dependencies(example ergovk)
target_link_libraries(example PRIVATE ergovk Vulkan::Vulkan Vulkan::shaderc_combined SDL2::SDL2main SDL2::SDL2-static)
target_include_directories(example PRIVATE ${ERGOVK_VULKAN_SDK}/include/vma)