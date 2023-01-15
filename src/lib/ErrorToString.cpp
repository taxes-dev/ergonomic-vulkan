#include "ergovk.hpp"
#include <unordered_map>

namespace
{
	const static std::unordered_map<ergovk::InitializeError, std::string_view> initialize_errors = {
		{ ergovk::InitializeError::AllocatorCreate, "Unable to create memory allocator" },
		{ ergovk::InitializeError::FailedCreate, "Unable to create instance" },
		{ ergovk::InitializeError::NoSuitableGpu, "No suitable GPU found" },
		{ ergovk::InitializeError::SurfaceCreate, "Unable to acquire surface" }
	};

	const static std::unordered_map<ergovk::SwapchainCreateError, std::string_view> swapchain_errors = {
		{ ergovk::SwapchainCreateError::DepthImageAllocation, "Unable to allocate depth buffer image" },
		{ ergovk::SwapchainCreateError::DepthImageViewAllocation, "Unable to allocate depth image view allocation" },
		{ ergovk::SwapchainCreateError::FailedCreate, "Unable to create swapchain" },
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

	std::string_view error_to_string(SwapchainCreateError error)
	{
		auto it = swapchain_errors.find(error);
		if (it != swapchain_errors.end())
		{
			return it->second;
		}
		assert(false);
		return "";
	}

}