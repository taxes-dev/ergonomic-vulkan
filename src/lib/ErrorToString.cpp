#include "ergovk.hpp"
#include <unordered_map>

namespace
{
	const static std::unordered_map<ergovk::InitializeError, std::string_view> initialize_errors = {
		{ ergovk::InitializeError::AllocatorCreate, "Unable to create memory allocator" },
		{ ergovk::InitializeError::CommandBufferCreate, "Unable to allocate a command buffer from the pool" },
		{ ergovk::InitializeError::CommandPoolCreate, "Unable to create command pool" },
		{ ergovk::InitializeError::NullCommandPool,
			"Attempt to create command buffer from an unitialized command pool" },
		{ ergovk::InitializeError::FailedCreate, "Unable to create instance" },
		{ ergovk::InitializeError::NoSuitableGpu, "No suitable GPU found" },
		{ ergovk::InitializeError::SurfaceCreate, "Unable to acquire surface" },
		{ ergovk::InitializeError::DepthImageAllocation, "Unable to allocate depth buffer image" },
		{ ergovk::InitializeError::DepthImageViewAllocation, "Unable to allocate depth image view allocation" },
		{ ergovk::InitializeError::RenderPassCreate, "Unable to create render pass" },
		{ ergovk::InitializeError::SwapchainCreate, "Unable to create swapchain" },
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
		assert(false);
		return "";
	}
}