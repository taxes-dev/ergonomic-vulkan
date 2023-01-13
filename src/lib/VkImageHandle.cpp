#include "ergovk.hpp"

namespace ergovk
{
	VkImageHandle::~VkImageHandle()
	{
		if (this->image)
		{
			vmaDestroyImage(this->m_allocator, this->image, this->allocation);
		}
	};

	Result<std::unique_ptr<VkImageHandle>, VkResult> VkImageHandle::create(VmaAllocator allocator, VkDevice device,
		VkImageCreateInfo create_info, VmaAllocationCreateInfo allocation_create_info)
	{

		VkImage image{ VK_NULL_HANDLE };
		VmaAllocation allocation{ VK_NULL_HANDLE };
		VkResult result =
			vmaCreateImage(allocator, &create_info, &allocation_create_info, &image, &allocation, nullptr);
		if (result == VK_SUCCESS)
		{
			auto vkimage = std::unique_ptr<VkImageHandle>(new VkImageHandle());
			vkimage->allocation = allocation;
			vkimage->image = image;
			vkimage->m_allocator = allocator;
			vkimage->m_device = device;
			return { std::move(vkimage) };
		}

		return { result };
	}

}