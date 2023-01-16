#include "ergovk.hpp"

namespace ergovk
{
	void RenderPass::destroy()
	{
		if (this->m_render_pass)
		{
			vkDestroyRenderPass(this->m_device, this->m_render_pass, nullptr);
			this->m_render_pass = VK_NULL_HANDLE;
		}
	}

	Result<RenderPass, InitializeError> RenderPass::create(RenderPassCreateInfo& create_info)
	{
		auto render_pass_create_info = create_info.value();
		VkRenderPass render_pass;
		VkResult result = vkCreateRenderPass(create_info.m_device, &render_pass_create_info, nullptr, &render_pass);
		if (result != VK_SUCCESS)
		{
			return InitializeError::RenderPassCreate;
		}
		return RenderPass{ create_info.m_device, render_pass };
	}

}