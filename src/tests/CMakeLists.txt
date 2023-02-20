add_executable(ergovk-tests
    ErrorTests.cpp
    ResultTests.cpp
    ResourceSetTests.cpp)
target_compile_features(ergovk-tests PUBLIC cxx_std_20)
add_dependencies(ergovk-tests ergovk)
target_link_libraries(ergovk-tests PRIVATE ergovk Vulkan::Vulkan Catch2::Catch2WithMain)
