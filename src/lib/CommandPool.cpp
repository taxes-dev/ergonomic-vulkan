#include "ergovk.hpp"

namespace ergovk
{
	CommandPool::~CommandPool()
	{
		if (this->m_command_pool)
		{
			vkDestroyCommandPool(this->m_device, this->m_command_pool, nullptr);
			this->m_command_pool = VK_NULL_HANDLE;
		}
	}

	Result<std::shared_ptr<CommandPool>, InitializeError> CommandPool::create(
		VulkanInstance& instance, CommandPoolCreateInfo create_info)
	{
		auto command_pool_info = ergovk::structs::create<VkCommandPoolCreateInfo>();
		command_pool_info.flags = create_info.create_flag_bits;
		command_pool_info.queueFamilyIndex = create_info.graphics_queue_family;
		VkCommandPool vk_command_pool;
		VkResult result = vkCreateCommandPool(instance.device, &command_pool_info, nullptr, &vk_command_pool);
		if (result != VK_SUCCESS)
		{
			return InitializeError::CommandPoolCreate;
		}
		auto command_pool = std::make_shared<CommandPool>(instance.device, vk_command_pool);
		instance.resources<CommandPool>().insert(create_info.resource_id, command_pool);
		return command_pool;
	}

	Result<VkCommandBuffer, InitializeError> CommandPool::create_command_buffer(VkCommandBufferLevel level)
	{
		if (this->m_command_pool)
		{
			auto command_buffer_alloc_info = structs::create<VkCommandBufferAllocateInfo>();
			command_buffer_alloc_info.commandPool = this->m_command_pool;
			command_buffer_alloc_info.level = level;
			VkCommandBuffer command_buffer;
			if (vkAllocateCommandBuffers(this->m_device, &command_buffer_alloc_info, &command_buffer) == VK_SUCCESS)
			{
				return command_buffer;
			}
			return InitializeError::CommandBufferCreate;
		}
		return InitializeError::NullCommandPool;
	}

}