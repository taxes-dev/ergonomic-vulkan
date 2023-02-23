#include "ergovk.hpp"

namespace ergovk
{
	FrameBuffer::~FrameBuffer()
	{
		if (this->m_frame_buffer)
		{
			vkDestroyFramebuffer(this->m_device, this->m_frame_buffer, nullptr);
		}
	}

	Result<std::shared_ptr<FrameBuffer>, InitializeError> FrameBuffer::create(
		VulkanInstance& instance, FrameBufferCreateInfo& create_info)
	{
		if (create_info.image_views.size() == 0 || create_info.render_pass == nullptr)
		{
			return InitializeError::RenderPassAndImageViewsRequired;
		}
		auto fb_info = structs::create<VkFramebufferCreateInfo>();
		fb_info.renderPass = create_info.render_pass->get_render_pass();
		fb_info.width = create_info.extents.width;
		fb_info.height = create_info.extents.height;
		fb_info.flags = create_info.flags;
		std::vector<VkImageView> image_views{};
		std::for_each(create_info.image_views.begin(), create_info.image_views.end(),
			[&image_views](
				std::shared_ptr<VkImageViewHandle> image_view) { image_views.push_back(image_view->image_view); });
		if (create_info.depth_buffer)
		{
			image_views.push_back(create_info.depth_buffer->image_view);
		}
		fb_info.attachmentCount = image_views.size();
		fb_info.pAttachments = image_views.data();

		VkFramebuffer vk_frame_buffer;
		VkResult result = vkCreateFramebuffer(instance.device, &fb_info, nullptr, &vk_frame_buffer);
		if (result != VK_SUCCESS)
		{
			return InitializeError::FrameBufferCreate;
		}

		auto frame_buffer = std::make_shared<FrameBuffer>(instance.device, vk_frame_buffer);
		instance.resources<FrameBuffer>().insert(create_info.resource_id, frame_buffer);
		return frame_buffer;
	}

}