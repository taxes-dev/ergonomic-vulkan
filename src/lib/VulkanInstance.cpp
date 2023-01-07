#include "ergovk.hpp"
#include "VkBootstrap.h"

namespace ergovk
{
    VulkanInstance::~VulkanInstance()
    {
        if (this->m_device)
        {
            vkDeviceWaitIdle(this->m_device);
        }
        if (this->m_allocator)
        {
            vmaDestroyAllocator(this->m_allocator);
        }
        if (this->m_device)
        {
            vkDestroyDevice(this->m_device, nullptr);
        }
        if (this->m_surface)
        {
            vkDestroySurfaceKHR(this->m_instance, this->m_surface, nullptr);
        }
        if (this->m_debug_messenger)
        {
            vkb::destroy_debug_utils_messenger(this->m_instance, this->m_debug_messenger, nullptr);
        }
        if (this->m_instance)
        {
            vkDestroyInstance(this->m_instance, nullptr);
        }
    }

}