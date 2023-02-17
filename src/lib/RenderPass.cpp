#include "ergovk.hpp"

namespace ergovk
{
	RenderPass::~RenderPass()
	{
		if (this->m_render_pass)
		{
			vkDestroyRenderPass(this->m_device, this->m_render_pass, nullptr);
			this->m_render_pass = VK_NULL_HANDLE;
		}
	}

	Result<std::shared_ptr<RenderPass>, InitializeError> RenderPass::create(VulkanInstance & instance, ResourceID resource_id, RenderPassCreateInfo& create_info)
	{
		auto render_pass_create_info = create_info.value();
		VkRenderPass vk_render_pass;
		VkResult result = vkCreateRenderPass(instance.device, &render_pass_create_info, nullptr, &vk_render_pass);
		if (result != VK_SUCCESS)
		{
			return InitializeError::RenderPassCreate;
		}
		auto render_pass = std::make_shared<RenderPass>( instance.device, vk_render_pass );
		instance.resources<RenderPass>().insert(resource_id, render_pass);
		return render_pass;
	}

}