#include "ergovk.hpp"
#include "VkBootstrap.h"

namespace ergovk
{
	void VulkanInstance::destroy()
	{
		if (this->device)
		{
			vkDeviceWaitIdle(this->device);
		}
		this->m_swapchain.destroy();
		if (this->m_allocator)
		{
			vmaDestroyAllocator(this->m_allocator);
			this->m_allocator = VK_NULL_HANDLE;
		}
		if (this->device)
		{
			vkDestroyDevice(this->device, nullptr);
			this->device = VK_NULL_HANDLE;
		}
		if (this->m_surface)
		{
			vkDestroySurfaceKHR(this->instance, this->m_surface, nullptr);
			this->m_surface = VK_NULL_HANDLE;
		}
		if (this->m_debug_messenger)
		{
			vkb::destroy_debug_utils_messenger(this->instance, this->m_debug_messenger, nullptr);
			this->m_debug_messenger = VK_NULL_HANDLE;
		}
		if (this->instance)
		{
			vkDestroyInstance(this->instance, nullptr);
			this->instance = VK_NULL_HANDLE;
		}
	}

}