#include <iostream>
#include "ergovk.hpp"
#include <SDL.h>
#include <SDL_vulkan.h>

inline constexpr std::int32_t kWindowWidth = 1200;
inline constexpr std::int32_t kWindowHeight = 800;

using namespace ergovk;

// used for validation layer messages
VkBool32 vk_debug_messenger(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
	VkDebugUtilsMessageTypeFlagsEXT message_type, const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data,
	void* p_user_data)
{
	switch (message_severity)
	{
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
		std::cerr << p_callback_data->pMessage << std::endl;
		break;
	default:
		std::cout << p_callback_data->pMessage << std::endl;
		break;
	}
	return VK_FALSE;
}

int main(int argc, char** argv)
{
	SDL_Init(SDL_INIT_VIDEO);
	SDL_Window* window = SDL_CreateWindow("Ergonomic Vulkan Example", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		kWindowWidth, kWindowHeight, SDL_WINDOW_VULKAN | SDL_WINDOW_ALLOW_HIGHDPI);
	if (window == nullptr)
	{
		std::cerr << SDL_GetError() << std::endl;
		return 1;
	}

	// update extents based on window size (may be different from requested)
	std::int32_t w, h;
	SDL_GetWindowSizeInPixels(window, &w, &h);
	VkExtent2D window_extent = { static_cast<std::uint32_t>(w), static_cast<std::uint32_t>(h) };

	try
	{
		// start up a Vulkan instance
		VulkanInstanceBuilder builder{};
		builder.set_custom_debug_callback(vk_debug_messenger)
			.set_create_surface_callback([=](VkInstance instance) -> VkSurfaceKHR {
				// instance needs a Surface, which SDL can create
				VkSurfaceKHR surface{ VK_NULL_HANDLE };
				if (SDL_Vulkan_CreateSurface(window, instance, &surface) == SDL_FALSE)
				{
					return VK_NULL_HANDLE;
				}
				// the instance will take ownership of the surface and destroy it later
				return surface;
			});
		auto instance_ret = builder.build();
		if (is_error(instance_ret))
		{
			std::cerr << "Error creating Vulkan instance: " << error_to_string(get_error(instance_ret)) << std::endl;
			return 1;
		}

		VulkanInstance instance = unwrap(instance_ret);

		std::cout << "GPU minimum buffer alignment of " << instance.get_min_uniform_buffer_offset_alignment()
				  << " byte(s)" << std::endl;

		// loop
		bool running{ true };
		while (running)
		{
			SDL_Event e{};
			while (SDL_PollEvent(&e) != 0)
			{
				if (e.type == SDL_QUIT)
				{
					running = false;
				}
				else if (e.type == SDL_KEYDOWN)
				{
					if (e.key.keysym.sym == SDLK_ESCAPE)
					{
						running = false;
					}
				}
			}
		}
	}
	catch (std::exception& e)
	{
		std::cerr << "Fatal: " << e.what() << std::endl;
		return 2;
	}

	// these are outside the main block so that SDL will be cleaned up after the VulkanInstance is done cleaning up
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}