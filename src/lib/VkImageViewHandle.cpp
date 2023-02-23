#include "ergovk.hpp"

namespace ergovk
{
	VkImageViewHandle::~VkImageViewHandle()
	{
		if (this->image_view)
		{
			vkDestroyImageView(this->m_device, this->image_view, nullptr);
			this->image_view = VK_NULL_HANDLE;
		}
	};

	Result<std::shared_ptr<VkImageViewHandle>, VkResult> VkImageViewHandle::create(
		VulkanInstance& instance, ResourceID resource_id, VkImageViewCreateInfo create_info)
	{
		VkImageView image_view{ VK_NULL_HANDLE };
		VkResult result = vkCreateImageView(instance.device, &create_info, nullptr, &image_view);
		if (result == VkResult::VK_SUCCESS)
		{
			auto handle = std::make_shared<VkImageViewHandle>(instance.device, image_view);
			instance.resources<VkImageViewHandle>().insert(resource_id, handle);
			return handle;
		}
		return result;
	}

}