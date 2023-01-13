#include "ergovk.hpp"
#include "VkBootstrap.h"

namespace ergovk
{
	VulkanInstance::~VulkanInstance()
	{
		if (this->device)
		{
			vkDeviceWaitIdle(this->device);
		}
        this->m_swapchain.reset();
		if (this->m_allocator)
		{
			vmaDestroyAllocator(this->m_allocator);
		}
		if (this->device)
		{
			vkDestroyDevice(this->device, nullptr);
		}
		if (this->m_surface)
		{
			vkDestroySurfaceKHR(this->instance, this->m_surface, nullptr);
		}
		if (this->m_debug_messenger)
		{
			vkb::destroy_debug_utils_messenger(this->instance, this->m_debug_messenger, nullptr);
		}
		if (this->instance)
		{
			vkDestroyInstance(this->instance, nullptr);
		}
	}

}