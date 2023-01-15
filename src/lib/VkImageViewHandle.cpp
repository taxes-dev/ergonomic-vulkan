#include "ergovk.hpp"

namespace ergovk
{
	void VkImageViewHandle::destroy()
	{
		if (this->image_view)
		{
			vkDestroyImageView(this->m_device, this->image_view, nullptr);
			this->image_view = VK_NULL_HANDLE;
		}
	};

	Result<VkImageViewHandle, VkResult> VkImageViewHandle::create(VkDevice device, VkImageViewCreateInfo create_info)
	{
		VkImageView image_view{ VK_NULL_HANDLE };
		VkResult result = vkCreateImageView(device, &create_info, nullptr, &image_view);
		if (result == VkResult::VK_SUCCESS)
		{
			return VkImageViewHandle{ device, image_view };
		}
		return result;
	}

}