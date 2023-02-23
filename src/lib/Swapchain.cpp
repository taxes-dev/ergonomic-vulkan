#include "ergovk.hpp"
#include "VkBootstrap.h"

namespace ergovk
{
	Swapchain::~Swapchain()
	{
		if (this->m_swapchain)
		{
			this->m_depth_image_view.reset();
			this->m_depth_image.reset();
			this->m_color_image_view.reset();
			this->m_color_image.reset();
			this->m_present_image_view.reset();
			vkDestroySwapchainKHR(this->m_device, this->m_swapchain, nullptr);
			this->m_swapchain = VK_NULL_HANDLE;
		}
	}

	Result<std::shared_ptr<Swapchain>, InitializeError> Swapchain::create(
		VulkanInstance& instance, SwapchainCreateInfo create_info)
	{
		// checks
		if (create_info.extent.height < 1 || create_info.extent.width < 1)
		{
			return InitializeError::InvalidExtents;
		}

		// build the swapchain
		vkb::SwapchainBuilder swapchain_builder{ instance.physical_device, instance.device, instance.surface };
		auto builder_ret = swapchain_builder.use_default_format_selection()
							   .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
							   .set_desired_extent(create_info.extent.width, create_info.extent.height)
							   .build();
		auto swapchain = std::make_shared<Swapchain>();
		if (builder_ret)
		{
			// populate swapchain
			vkb::Swapchain vswapchain = builder_ret.value();
			swapchain->m_device = instance.device;
			swapchain->m_swapchain = vswapchain.swapchain;
			swapchain->m_images = vswapchain.get_images().value();
			swapchain->m_present_image_view =
				std::make_shared<VkImageViewHandle>(instance.device, vswapchain.get_image_views().value()[0]);
			ResourceID present_iv_resource_id{ create_info.resource_id + "__present_image_view" };
			instance.resources<VkImageViewHandle>().insert(present_iv_resource_id, swapchain->m_present_image_view);
			swapchain->m_image_format = vswapchain.image_format;
		}
		else
		{
			return InitializeError::SwapchainCreate;
		}

		// create custom image & view for multisampling
		auto image_create_info = structs::create<VkImageCreateInfo>();
		image_create_info.format = swapchain->m_image_format;
		image_create_info.extent = VkExtent3D{ create_info.extent.width, create_info.extent.height, 1 };
		image_create_info.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		image_create_info.samples = create_info.sample_count;

		// allocate color image
		VmaAllocationCreateInfo cimg_allocinfo{};
		cimg_allocinfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
		cimg_allocinfo.requiredFlags = VkMemoryPropertyFlags{ VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT };

		ResourceID color_img_resource_id{ create_info.resource_id + "__color_image" };
		auto color_image_ret =
			VkImageHandle::create(instance, color_img_resource_id, image_create_info, cimg_allocinfo);
		if (is_error(color_image_ret))
		{
			return InitializeError::ColorImageAllocation;
		}
		swapchain->m_color_image = unwrap(color_image_ret);

		VkImageView vk_color_view;
		auto image_view_create_info = structs::create<VkImageViewCreateInfo>();
		image_view_create_info.image = swapchain->m_color_image->image;
		image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		image_view_create_info.format = swapchain->m_image_format;
		image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		image_view_create_info.subresourceRange.levelCount = 1;
		image_view_create_info.subresourceRange.layerCount = 1;

		ResourceID color_iv_resource_id{ create_info.resource_id + "__color_image_view" };
		auto color_image_view_ret = VkImageViewHandle::create(instance, color_iv_resource_id, image_view_create_info);
		if (is_error(color_image_view_ret))
		{
			return InitializeError::ColorImageViewAllocation;
		}
		swapchain->m_color_image_view = unwrap(color_image_view_ret);


		if (create_info.create_depth_buffer)
		{
			// depth buffer
			swapchain->m_depth_format = VkFormat::VK_FORMAT_D32_SFLOAT;
			auto depth_image_info = structs::create<VkImageCreateInfo>();
			depth_image_info.format = swapchain->m_depth_format;
			depth_image_info.extent = VkExtent3D{ create_info.extent.width, create_info.extent.height, 1 };
			depth_image_info.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			depth_image_info.samples = create_info.sample_count;

			// allocate depth image
			VmaAllocationCreateInfo dimg_allocinfo{};
			dimg_allocinfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
			dimg_allocinfo.requiredFlags = VkMemoryPropertyFlags{ VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT };

			ResourceID depth_img_resource_id{ create_info.resource_id + "__depth_buffer_image" };
			auto depth_img_ret =
				VkImageHandle::create(instance, depth_img_resource_id, depth_image_info, dimg_allocinfo);
			if (is_error(depth_img_ret))
			{
				return InitializeError::DepthImageAllocation;
			}
			swapchain->m_depth_image = unwrap(depth_img_ret);

			// build an image view for rendering
			auto depth_imageview_info = structs::create<VkImageViewCreateInfo>();
			depth_imageview_info.format = swapchain->m_depth_format;
			depth_imageview_info.image = swapchain->m_depth_image->image;
			depth_imageview_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

			ResourceID depth_iv_resource_id{ create_info.resource_id + "__depth_buffer_image_view" };

			auto depth_imageview_ret = VkImageViewHandle::create(instance, depth_iv_resource_id, depth_imageview_info);
			if (is_error(depth_imageview_ret))
			{
				return InitializeError::DepthImageViewAllocation;
			}
			swapchain->m_depth_image_view = unwrap(depth_imageview_ret);
		}

		instance.resources<Swapchain>().insert(create_info.resource_id, swapchain);
		return swapchain;
	}

}