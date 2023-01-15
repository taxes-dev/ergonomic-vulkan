#include "ergovk.hpp"

namespace ergovk
{
	void VkImageHandle::destroy()
	{
		if (this->image)
		{
			vmaDestroyImage(this->m_allocator, this->image, this->allocation);
			this->image = VK_NULL_HANDLE;
			this->allocation = VK_NULL_HANDLE;
		}
	};

	Result<VkImageHandle, VkResult> VkImageHandle::create(VmaAllocator allocator, VkDevice device,
		VkImageCreateInfo create_info, VmaAllocationCreateInfo allocation_create_info)
	{
		VkImage image{ VK_NULL_HANDLE };
		VmaAllocation allocation{ VK_NULL_HANDLE };
		VkResult result =
			vmaCreateImage(allocator, &create_info, &allocation_create_info, &image, &allocation, nullptr);
		if (result == VK_SUCCESS)
		{
			return VkImageHandle{ allocator, device, image, allocation };
		}

		return result;
	}

}