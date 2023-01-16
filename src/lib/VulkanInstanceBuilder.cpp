#include "ergovk.hpp"
#include "VkBootstrap.h"

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
			if (is_error(swapchain_ret))
			{
				return get_error(swapchain_ret);
			}
			instance.m_swapchain = unwrap(swapchain_ret);

			// create render frames and command pools
			instance.m_frames.resize(this->m_render_frames);
			CommandPoolCreateInfo command_pool_create_info{
				.device = instance.device,
				.graphics_queue_family = instance.m_graphics_queue_family,
				.create_flag_bits = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
			};
			for (auto & frame : instance.m_frames)
			{
				auto command_pool = CommandPool::create(command_pool_create_info);
				if (is_error(command_pool))
				{
					return get_error(command_pool);
				}
				frame.command_pool = unwrap(command_pool);
				auto main_command_buffer = frame.command_pool.create_command_buffer();
				if (is_error(main_command_buffer))
				{
					return get_error(main_command_buffer);
				}
				frame.command_buffer = get_value(main_command_buffer);
			}

			return instance;
		}
		return InitializeError::FailedCreate;
	}
}