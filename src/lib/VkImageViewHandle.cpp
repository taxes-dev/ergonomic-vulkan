#include "ergovk.hpp"

namespace ergovk
{
	VkImageViewHandle::~VkImageViewHandle()
	{
		if (this->image_view)
		{
			vkDestroyImageView(this->m_device, this->image_view, nullptr);
		}
	};

	Result<std::unique_ptr<VkImageViewHandle>, VkResult> VkImageViewHandle::create(
		VkDevice device, VkImageViewCreateInfo create_info)
	{
		VkImageView image_view{ VK_NULL_HANDLE };
		VkResult result = vkCreateImageView(device, &create_info, nullptr, &image_view);
		if (result == VkResult::VK_SUCCESS)
		{
			auto vkimage_view = std::unique_ptr<VkImageViewHandle>(new VkImageViewHandle());
			vkimage_view->m_device = device;
			vkimage_view->image_view = image_view;
			return { std::move(vkimage_view) };
		}
		return { result };
	}

}