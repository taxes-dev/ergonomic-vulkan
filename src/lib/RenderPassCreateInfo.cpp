#include "ergovk.hpp"

namespace ergovk
{
	const VkRenderPassCreateInfo& RenderPassCreateInfo::value()
	{
		this->m_create_info.flags = this->m_create_flag_bits;
		if (this->m_create_info.attachmentCount != this->m_attachment_descriptions.size())
		{
			this->m_create_info.attachmentCount = this->m_attachment_descriptions.size();
			this->m_create_info.pAttachments = this->m_attachment_descriptions.data();
		}
		if (this->m_create_info.dependencyCount != this->m_subpass_dependencies.size())
		{
			this->m_create_info.dependencyCount = this->m_subpass_dependencies.size();
			this->m_create_info.pDependencies = this->m_subpass_dependencies.data();
		}
		if (this->m_create_info.subpassCount != this->m_subpass_descriptions.size())
		{
			this->m_create_info.subpassCount = this->m_subpass_descriptions.size();
			this->m_create_info.pSubpasses = this->m_subpass_descriptions.data();
		}

		return this->m_create_info;
	}

}