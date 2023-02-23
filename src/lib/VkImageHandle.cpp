#include "ergovk.hpp"

namespace ergovk
{
	VkImageHandle::~VkImageHandle()
	{
		if (this->image)
		{
			vmaDestroyImage(this->m_allocator, this->image, this->allocation);
			this->image = VK_NULL_HANDLE;
			this->allocation = VK_NULL_HANDLE;
		}
	};

	Result<std::shared_ptr<VkImageHandle>, VkResult> VkImageHandle::create(VulkanInstance& instance,
		ResourceID resource_id, VkImageCreateInfo create_info, VmaAllocationCreateInfo allocation_create_info)
	{
		VkImage image{ VK_NULL_HANDLE };
		VmaAllocation allocation{ VK_NULL_HANDLE };
		VkResult result =
			vmaCreateImage(instance.allocator, &create_info, &allocation_create_info, &image, &allocation, nullptr);
		if (result == VK_SUCCESS)
		{
			auto handle = std::make_shared<VkImageHandle>(instance.allocator, instance.device, image, allocation);
			instance.resources<VkImageHandle>().insert(resource_id, handle);
			return handle;
		}

		return result;
	}

}