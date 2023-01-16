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
	 * @param device VkDevice
	 * @param swapchain_image_format VkFormat
	 * @param depth_buffer_image_format VkFormat (if undefined, the depth buffer will be skipped)
	*/
	Result<RenderPass, InitializeError> create_render_pass(
		VkDevice device, VkFormat swapchain_image_format, VkFormat depth_buffer_image_format)
	{

		RenderPassCreateInfo create_info{ device };
		VkAttachmentDescription color_attachment{};
		color_attachment.format = swapchain_image_format;
		// 1 sample, no MSAA
		color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
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
		depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
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

		return RenderPass::create(create_info);
	}
}

namespace ergovk
{
	VulkanInstanceBuilder::VulkanInstanceBuilder() noexcept
	{
		this->m_builder = new vkb::InstanceBuilder();
		this->m_builder->require_api_version(1, 1, 0);
#ifdef ERGOVK_DEBUG
		this->m_builder->request_validation_layers(true);
		this->m_builder->use_default_debug_messenger();
#endif
		this->m_create_surface_callback = [](VkInstance) { return VK_NULL_HANDLE; };
	}

	VulkanInstanceBuilder::~VulkanInstanceBuilder()
	{
		if (this->m_builder)
		{
			delete this->m_builder;
		}
	}

	VulkanInstanceBuilder& VulkanInstanceBuilder::set_create_surface_callback(CreateSurfaceCallback&& callback)
	{
		this->m_create_surface_callback = std::forward<CreateSurfaceCallback>(callback);
		return *this;
	}

	VulkanInstanceBuilder& VulkanInstanceBuilder::set_custom_debug_callback(
		PFN_vkDebugUtilsMessengerCallbackEXT callback)
	{
		this->m_builder->set_debug_callback(callback);
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
		auto build_ret = this->m_builder->build();
		if (build_ret)
		{
			VulkanInstance instance{};
			instance.m_debug_messenger = build_ret->debug_messenger;
			instance.instance = build_ret->instance;

			// create surface
			instance.m_surface = this->m_create_surface_callback(instance.instance);
			if (instance.m_surface == VK_NULL_HANDLE)
			{
				return InitializeError::SurfaceCreate;
			}

			// find a suitable GPU
			// TODO: allow selection of a specific GPU
			vkb::PhysicalDeviceSelector selector{ *build_ret };
			auto selector_ret = selector							 //
									.set_minimum_version(1, 1)		 //
									.set_surface(instance.m_surface) //
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
				instance.m_physical_device = selector_ret->physical_device;

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

			// initialize memory alloactor
			VmaAllocatorCreateInfo allocator_info{};
			allocator_info.physicalDevice = instance.m_physical_device;
			allocator_info.device = instance.device;
			allocator_info.instance = instance.instance;

			if (vmaCreateAllocator(&allocator_info, &instance.m_allocator) != VK_SUCCESS)
			{
				return InitializeError::AllocatorCreate;
			}

			// create swapchain
			SwapchainCreateInfo swapchain_create_info{
				.allocator = instance.m_allocator,
				.physical_device = instance.m_physical_device,
				.device = instance.device,
				.surface = instance.m_surface,
				.extent = this->m_extent,
				.create_depth_buffer = this->m_create_depth_buffer,
			};
			auto swapchain_ret = Swapchain::create(swapchain_create_info);
			RETURN_IF_ERROR(swapchain_ret);
			instance.m_swapchain = unwrap(swapchain_ret);

			// create render frames and command pools
			instance.m_frames.resize(this->m_render_frames);
			CommandPoolCreateInfo command_pool_create_info{
				.device = instance.device,
				.graphics_queue_family = instance.m_graphics_queue_family,
				.create_flag_bits = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
			};
			for (auto& frame : instance.m_frames)
			{
				auto command_pool = CommandPool::create(command_pool_create_info);
				RETURN_IF_ERROR(command_pool);
				frame.command_pool = unwrap(command_pool);
				auto main_command_buffer = frame.command_pool.create_command_buffer();
				RETURN_IF_ERROR(main_command_buffer);
				frame.command_buffer = get_value(main_command_buffer);
			}

			// create a separate command pool for "immediate" executed commands
			// this is useful for uploading textures/meshes to the GPU
			command_pool_create_info.create_flag_bits = static_cast<VkCommandPoolCreateFlagBits>(0);
			auto command_pool = CommandPool::create(command_pool_create_info);
			RETURN_IF_ERROR(command_pool);
			instance.m_immediate_command_pool = unwrap(command_pool);
			auto immed_command_buffer = instance.m_immediate_command_pool.create_command_buffer();
			RETURN_IF_ERROR(immed_command_buffer);
			instance.m_immediate_command_buffer = get_value(immed_command_buffer);

			// create the default render pass
			auto render_pass = create_render_pass(instance.device, instance.m_swapchain.get_image_format(),
				instance.m_swapchain.get_depth_buffer_image_format());
			RETURN_IF_ERROR(render_pass);
			instance.m_render_pass = unwrap(render_pass);

			return instance;
		}
		return InitializeError::FailedCreate;
	}
}