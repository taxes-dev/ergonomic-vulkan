#include "ergovk.hpp"
#include "VkBootstrap.h"

namespace ergovk
{
	void VulkanInstance::destroy()
	{
		this->wait_for_idle();
		this->m_render_pass.destroy();
		this->m_immediate_command_pool.destroy();
		this->m_frames.clear();
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