# Ergonomic Vulkan Library

This project has a few very specific goals:
* Create an easy bootstrap for Vulkan, in an opinionated manner
* Cross-platform C++20
* Cover the most common needs for a program which:
    * Renders images to an OS window/surface
    * Uses typical meshes and shaders

What this project is NOT:
* An encapsulation/wrapper for the entire Vulkan API
* Flexible for every possible Vulkan use case

## Building

You will need:
* CMake 3.24+
* A C++ compiler that supports C++20
* The [Vulkan SDK](https://vulkan.lunarg.com/sdk/home) with VMA and shaderc_combined

If you're building the example program, you'll also need:
* [SDL2](https://wiki.libsdl.org/SDL2/Installation)

Running CMake with the root of the repo as the source dir will configure everything for compilation. There are a few variables to be aware of:
* `VULKAN_SDK` - this environment variable should be set by the SDK's `setup-env.sh` script
    * Alternatively, you can set `ERGOVK_VULKAN_SDK` explicitly in the cache
* `ERGOVK_BUILD_EXAMPLE` - if this cache var is `TRUE` (the default), the example program is built in addition to the library

## License

Distributed under an [MIT license](LICENSE.md). You're welcome to copy, modify, and distribute, so long as you follow the rules of the license.

vk-bootstrap distrubted under [MIT license](https://github.com/charles-lunarg/vk-bootstrap/blob/master/LICENSE.txt).

Vulkan/VMA and other dependencies have their own licenses.