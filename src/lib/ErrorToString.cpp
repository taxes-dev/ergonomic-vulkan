#include "ergovk.hpp"
#include <unordered_map>

namespace
{
	using ergovk::InitializeError;
	const static std::unordered_map<InitializeError, std::string_view> initialize_errors = {
		{ InitializeError::AllocatorCreate, "Unable to create memory allocator" },
		{ InitializeError::CommandBufferCreate, "Unable to allocate a command buffer from the pool" },
		{ InitializeError::CommandPoolCreate, "Unable to create command pool" },
		{ InitializeError::NullCommandPool, "Attempt to create command buffer from an unitialized command pool" },
		{ InitializeError::FailedCreate, "Unable to create instance" },
		{ InitializeError::NoSuitableGpu, "No suitable GPU found" },
		{ InitializeError::SurfaceCreate, "Unable to acquire surface" },
		{ InitializeError::DepthImageAllocation, "Unable to allocate depth buffer image" },
		{ InitializeError::DepthImageViewAllocation, "Unable to allocate depth image view allocation" },
		{ InitializeError::RenderPassCreate, "Unable to create render pass" },
		{ InitializeError::SwapchainCreate, "Unable to create swapchain" },
	};

	using ergovk::resources::ResourceError;
	const static std::unordered_map<ResourceError, std::string_view> resource_errors = {
		{ ResourceError::ResourceNotPresent,
			"The resource with the given resource ID isn't present in the resource set" },
	};
}

namespace ergovk
{
	std::string_view error_to_string(InitializeError error)
	{
		auto it = initialize_errors.find(error);
		if (it != initialize_errors.end())
		{
			return it->second;
		}
		return "";
	}

	std::string_view error_to_string(resources::ResourceError error)
	{
		auto it = resource_errors.find(error);
		if (it != resource_errors.end())
		{
			return it->second;
		}
		return "";
	}
}