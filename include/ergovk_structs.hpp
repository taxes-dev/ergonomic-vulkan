#pragma once
#include <vulkan/vulkan.h>

namespace ergovk::structs
{
	/**
     * @brief Create a Vulkan struct of type \p T with reasonable defaults.
     * @tparam T a Vulkan struct to create
     * @note This template must be explicitly specialized for the different structs.
    */
	template <typename T>
	T create();

	template <>
	inline VkCommandPoolCreateInfo create()
	{
		return VkCommandPoolCreateInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .queueFamilyIndex = 0,
        };
	};

	template <>
	inline VkPhysicalDeviceShaderDrawParametersFeatures create()
	{
		return VkPhysicalDeviceShaderDrawParametersFeatures{
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETER_FEATURES,
			.pNext = nullptr,
			.shaderDrawParameters = VK_TRUE,
		};
	};

}