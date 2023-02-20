#include "ergovk.hpp"
#include "VkBootstrap.h"

// common pattern in build() function below
#define RETURN_IF_ERROR(x)                                                                                             \
	if (is_error(x))                                                                                                   \
	{                                                                                                                  \
		return get_error(x);                                                                                           \
	}

namespace
{
	using namespace ergovk;

	/**
	 * @brief Used by ergovk::VulkanInstanceBuilder::build() to create a default render pass.
	 * @param instance ergovk::VulkanInstance
	 * @param swapchain_image_format VkFormat
	 * @param depth_buffer_image_format VkFormat (if undefined, the depth buffer will be skipped)
	 * @param sample_count VkSampleCountFlagBits
	 * @returns std::shared_ptr<ergovk::RenderPass> or ergovk::InitializeError
	*/
	Result<std::shared_ptr<RenderPass>, InitializeError> create_render_pass(
		VulkanInstance & instance, VkFormat swapchain_image_format, VkFormat depth_buffer_image_format,
		VkSampleCountFlagBits sample_count)
	{

		RenderPassCreateInfo create_info{};
		VkAttachmentDescription color_attachment{};
		color_attachment.format = swapchain_image_format;
		// 1 sample, no MSAA
		color_attachment.samples = sample_count;
		// clear when attachment is loaded
		color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		// store when renderpass ends
		color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		// don't care about stencils
		color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		// also don't care
		color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		// at end of renderpass, should be ready to display
		color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference color_attachment_ref{};
		// attachment number will index into pAttachments in the parent renderpass
		color_attachment_ref.attachment = 0;
		color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		// attach to create info
		create_info.add_attachment_description(color_attachment);

		// for depth buffer
		VkAttachmentDescription depth_attachment{};
		depth_attachment.flags = 0;
		depth_attachment.format = depth_buffer_image_format;
		depth_attachment.samples = sample_count;
		depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depth_attachment_ref{};
		depth_attachment_ref.attachment = 1;
		depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		// attach to create info
		if (depth_buffer_image_format != VK_FORMAT_UNDEFINED)
		{
			create_info.add_attachment_description(depth_attachment);
		}

		// create 1 subpass
		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &color_attachment_ref;
		if (depth_buffer_image_format != VK_FORMAT_UNDEFINED)
		{
			subpass.pDepthStencilAttachment = &depth_attachment_ref;
		}

		// attach to create info
		create_info.add_subpass_description(subpass);

		// dependencies
		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		create_info.add_subpass_dependency(dependency);


		if (depth_buffer_image_format != VK_FORMAT_UNDEFINED)
		{
			VkSubpassDependency depth_dependency{};
			depth_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			depth_dependency.dstSubpass = 0;
			depth_dependency.srcStageMask =
				VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			depth_dependency.srcAccessMask = 0;
			depth_dependency.dstStageMask =
				VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			depth_dependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			create_info.add_subpass_dependency(depth_dependency);
		}

		return RenderPass::create(instance, defaults::RESID_DEFAULT_RENDERPASS,create_info);
	}

	VkSampleCountFlagBits get_desired_sample_count(
		const VkPhysicalDeviceProperties& props, DesiredPerPixelSampling desired_count)
	{
		VkSampleCountFlags counts =
			props.limits.framebufferColorSampleCounts & props.limits.framebufferDepthSampleCounts;
		if ((counts & VK_SAMPLE_COUNT_64_BIT) && desired_count >= DesiredPerPixelSampling::Sample_64Bit)
		{
			return VK_SAMPLE_COUNT_64_BIT;
		}
		if ((counts & VK_SAMPLE_COUNT_32_BIT) && desired_count >= DesiredPerPixelSampling::Sample_32Bit)
		{
			return VK_SAMPLE_COUNT_32_BIT;
		}
		if ((counts & VK_SAMPLE_COUNT_16_BIT) && desired_count >= DesiredPerPixelSampling::Sample_16Bit)
		{
			return VK_SAMPLE_COUNT_16_BIT;
		}
		if ((counts & VK_SAMPLE_COUNT_8_BIT) && desired_count >= DesiredPerPixelSampling::Sample_8Bit)
		{
			return VK_SAMPLE_COUNT_8_BIT;
		}
		if ((counts & VK_SAMPLE_COUNT_4_BIT) && desired_count >= DesiredPerPixelSampling::Sample_4Bit)
		{
			return VK_SAMPLE_COUNT_4_BIT;
		}
		if ((counts & VK_SAMPLE_COUNT_2_BIT) && desired_count >= DesiredPerPixelSampling::Sample_2Bit)
		{
			return VK_SAMPLE_COUNT_2_BIT;
		}
		return VK_SAMPLE_COUNT_1_BIT;
	}
}

namespace ergovk
{
	VulkanInstanceBuilder::VulkanInstanceBuilder() noexcept
	{
		this->m_create_surface_callback = [](VkInstance) { return VK_NULL_HANDLE; };
	}

	VulkanInstanceBuilder::~VulkanInstanceBuilder()
	{
	}

	VulkanInstanceBuilder& VulkanInstanceBuilder::set_create_surface_callback(CreateSurfaceCallback&& callback)
	{
		this->m_create_surface_callback = std::forward<CreateSurfaceCallback>(callback);
		return *this;
	}

	VulkanInstanceBuilder& VulkanInstanceBuilder::set_custom_debug_callback(
		PFN_vkDebugUtilsMessengerCallbackEXT callback)
	{
		this->m_debug_callback = callback;
		return *this;
	}

	VulkanInstanceBuilder& VulkanInstanceBuilder::set_draw_extent(VkExtent2D extent)
	{
		assert(extent.height > 0);
		assert(extent.width > 0);
		this->m_extent = extent;
		return *this;
	}

	Result<VulkanInstance, InitializeError> VulkanInstanceBuilder::build() const
	{
		vkb::InstanceBuilder vkb_builder{};
		vkb_builder.require_api_version(1, 1, 0);
#ifdef ERGOVK_DEBUG
		vkb_builder.request_validation_layers(true);
		vkb_builder.use_default_debug_messenger();
#endif
		if (this->m_debug_callback)
		{
			vkb_builder.set_debug_callback(this->m_debug_callback);
		}

		auto build_ret = vkb_builder.build();
		if (build_ret)
		{
			VulkanInstance instance{};
			instance.m_debug_messenger = build_ret->debug_messenger;
			instance.instance = build_ret->instance;

			// create surface
			instance.surface = this->m_create_surface_callback(instance.instance);
			if (instance.surface == VK_NULL_HANDLE)
			{
				return InitializeError::SurfaceCreate;
			}

			// find a suitable GPU
			// TODO: allow selection of a specific GPU
			vkb::PhysicalDeviceSelector selector{ *build_ret };
			auto selector_ret = selector							 //
									.set_minimum_version(1, 1)		 //
									.set_surface(instance.surface) //
									.select();
			if (!selector_ret)
			{
				return InitializeError::NoSuitableGpu;
			}

			// create the device
			vkb::DeviceBuilder device_builder{ selector_ret.value() };
			auto shader_draw_features = structs::create<VkPhysicalDeviceShaderDrawParametersFeatures>();

			auto device_ret = device_builder.add_pNext(&shader_draw_features).build();
			if (device_ret)
			{
				instance.device = device_ret->device;
				instance.physical_device = selector_ret->physical_device;

				// get gpu properties
				instance.m_physical_device_properties = device_ret->physical_device.properties;

				// get queues
				auto queue_ret = device_ret->get_queue(vkb::QueueType::graphics);
				if (queue_ret)
				{
					instance.m_graphics_queue = *queue_ret;
					instance.m_graphics_queue_family = device_ret->get_queue_index(vkb::QueueType::graphics).value();
				}
				else
				{
					return InitializeError::NoSuitableGpu;
				}
			}
			else
			{
				return InitializeError::NoSuitableGpu;
			}

			// setup sampler count
			instance.m_sample_count = get_desired_sample_count(instance.m_physical_device_properties, this->m_samples);

			// initialize memory alloactor
			VmaAllocatorCreateInfo allocator_info{};
			allocator_info.physicalDevice = instance.physical_device;
			allocator_info.device = instance.device;
			allocator_info.instance = instance.instance;

			if (vmaCreateAllocator(&allocator_info, &instance.allocator) != VK_SUCCESS)
			{
				return InitializeError::AllocatorCreate;
			}

			// create swapchain
			SwapchainCreateInfo swapchain_create_info{
				.extent = this->m_extent,
				.create_depth_buffer = this->m_create_depth_buffer,
				.resource_id = defaults::RESID_SWAPCHAIN,
			};
			auto swapchain_ret = Swapchain::create(instance, swapchain_create_info);
			RETURN_IF_ERROR(swapchain_ret);

			// create render frames and command pools
			instance.m_frames.resize(this->m_render_frames);
			CommandPoolCreateInfo command_pool_create_info{
				.graphics_queue_family = instance.m_graphics_queue_family,
				.create_flag_bits = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
			};
			for (std::size_t frame_idx = 0; frame_idx < instance.m_frames.size(); frame_idx++)
			{
				command_pool_create_info.resource_id = defaults::RESID_RENDERFRAME_COMMANDPOOL;
				command_pool_create_info.resource_id += std::to_string(frame_idx);
				auto command_pool = CommandPool::create(instance, command_pool_create_info);
				RETURN_IF_ERROR(command_pool);
				instance.m_frames[frame_idx].command_pool = unwrap(command_pool);
				auto main_command_buffer = instance.m_frames[frame_idx].command_pool->create_command_buffer();
				RETURN_IF_ERROR(main_command_buffer);
				instance.m_frames[frame_idx].command_buffer = get_value(main_command_buffer);
			}

			// create a separate command pool for "immediate" executed commands
			// this is useful for uploading textures/meshes to the GPU
			command_pool_create_info.create_flag_bits = static_cast<VkCommandPoolCreateFlagBits>(0);
			command_pool_create_info.resource_id = defaults::RESID_RENDERFRAME_COMMANDPOOL;
			auto command_pool = CommandPool::create(instance, command_pool_create_info);
			RETURN_IF_ERROR(command_pool);
			auto immed_command_buffer = get_value(command_pool)->create_command_buffer();
			RETURN_IF_ERROR(immed_command_buffer);
			instance.m_immediate_command_buffer = get_value(immed_command_buffer);

			// create the default render pass
			auto render_pass = create_render_pass(instance, get_value(swapchain_ret)->get_image_format(),
				get_value(swapchain_ret)->get_depth_buffer_image_format(), instance.m_sample_count);
			RETURN_IF_ERROR(render_pass);

			return instance;
		}
		return InitializeError::FailedCreate;
	}
}