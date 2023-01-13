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
			this->m_image_views.clear();
			vkDestroySwapchainKHR(this->m_device, this->m_swapchain, nullptr);
		}
	}

	Result<std::unique_ptr<Swapchain>, SwapchainCreateError> Swapchain::create(VmaAllocator allocator,
		VkPhysicalDevice physical_device, VkDevice device, VkSurfaceKHR surface, VkExtent2D extent)
	{
		// build the swapchain
		vkb::SwapchainBuilder swapchain_builder{ physical_device, device, surface };
		auto builder_ret = swapchain_builder.use_default_format_selection()
							   .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
							   .set_desired_extent(extent.width, extent.height)
							   .build();
		auto swapchain = std::unique_ptr<Swapchain>(new Swapchain{});
		if (builder_ret)
		{
			vkb::Swapchain vswapchain = builder_ret.value();
			swapchain->m_device = device;
			swapchain->m_swapchain = vswapchain.swapchain;
			swapchain->m_images = vswapchain.get_images().value();
			// value() is creating the views on the fly, thus why we can't use it as an rvalue in the loop
			auto views = vswapchain.get_image_views().value();
			for (auto& image_view : views)
			{
				swapchain->m_image_views.emplace_back(device, image_view);
			}
			swapchain->m_image_format = vswapchain.image_format;
		}
		else
		{
			return { SwapchainCreateError::FailedCreate };
		}
		return { std::move(swapchain) };

		// depth buffer
		swapchain->m_depth_format = VkFormat::VK_FORMAT_D32_SFLOAT;
		auto depth_image_info = structs::create<VkImageCreateInfo>();
		depth_image_info.format = swapchain->m_depth_format;
		depth_image_info.extent = VkExtent3D{ extent.width, extent.height, 1 };
		depth_image_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

		// allocate depth image
		VmaAllocationCreateInfo dimg_allocinfo{};
		dimg_allocinfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
		dimg_allocinfo.requiredFlags = VkMemoryPropertyFlags{ VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT };

		auto depth_img_ret = VkImageHandle::create(allocator, device, depth_image_info, dimg_allocinfo);
		if (is_error(depth_img_ret))
		{
			return { SwapchainCreateError::DepthImageAllocation };
		}
		swapchain->m_depth_image = unwrap(depth_img_ret);

		// build an image view for rendering
		auto depth_imageview_info = structs::create<VkImageViewCreateInfo>();
		depth_imageview_info.format = swapchain->m_depth_format;
		depth_imageview_info.image = swapchain->m_depth_image->image;
		depth_imageview_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

		auto depth_imageview_ret = VkImageViewHandle::create(device, depth_imageview_info);
		if (is_error(depth_imageview_ret))
		{
			return { SwapchainCreateError::DepthImageViewAllocation };
		}
		swapchain->m_depth_image_view = unwrap(depth_imageview_ret);

		return { std::move(swapchain) };
	}

}