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
			.pNext = VK_NULL_HANDLE,
			.flags = 0,
			.queueFamilyIndex = 0,
		};
	};

	template <>
	inline VkImageCreateInfo create()
	{
		return VkImageCreateInfo{
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.pNext = VK_NULL_HANDLE,
			.imageType = VK_IMAGE_TYPE_2D,
			.mipLevels = 1,
			.arrayLayers = 1,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.tiling = VK_IMAGE_TILING_OPTIMAL,
		};
	};

	template <>
	inline VkImageViewCreateInfo create()
	{
		return VkImageViewCreateInfo{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.pNext = VK_NULL_HANDLE,
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.subresourceRange = {
                .baseMipLevel = 0,
			    .levelCount = 1,
			    .baseArrayLayer = 0,
			    .layerCount = 1,
            },
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