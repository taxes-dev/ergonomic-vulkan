#include "ergovk.hpp"
#include "VkBootstrap.h"

namespace ergovk
{
    VulkanInstanceBuilder::VulkanInstanceBuilder()
    {
        this->m_builder = new vkb::InstanceBuilder();
        this->m_builder->require_api_version(1, 1, 0);
#ifdef ERGOVK_DEBUG
        this->m_builder->request_validation_layers(true);
        this->m_builder->use_default_debug_messenger();
#endif
        this->m_create_surface_callback = [](VkInstance)
        { return VK_NULL_HANDLE; };
    }

    VulkanInstanceBuilder::~VulkanInstanceBuilder()
    {
        if (this->m_builder)
        {
            delete this->m_builder;
        }
    }

    VulkanInstanceBuilder &VulkanInstanceBuilder::set_create_surface_callback(CreateSurfaceCallback &&callback)
    {
        this->m_create_surface_callback = std::forward<CreateSurfaceCallback>(callback);
        return *this;
    }

    VulkanInstanceBuilder &VulkanInstanceBuilder::set_custom_debug_callback(PFN_vkDebugUtilsMessengerCallbackEXT callback)
    {
        this->m_builder->set_debug_callback(callback);
        return *this;
    }

    Result<std::unique_ptr<VulkanInstance>,InitializeError> VulkanInstanceBuilder::build() const
    {
        auto build_ret = this->m_builder->build();
        if (build_ret)
        {
            auto instance = std::unique_ptr<VulkanInstance>(new VulkanInstance());
            instance->m_debug_messenger = build_ret->debug_messenger;
            instance->m_instance = build_ret->instance;

            // create surface
            instance->m_surface = this->m_create_surface_callback(instance->m_instance);
            if (instance->m_surface == VK_NULL_HANDLE)
            {
                return {InitializeError::SurfaceCreate};
            }

            // find a suitable GPU
            vkb::PhysicalDeviceSelector selector{*build_ret};
            auto selector_ret = selector
                                    .set_minimum_version(1, 1)
                                    .set_surface(instance->m_surface)
                                    .select();
            if (!selector_ret)
            {
                return {InitializeError::NoSuitableGpu};
            }

            // create the device
            vkb::DeviceBuilder device_builder{selector_ret.value()};
            VkPhysicalDeviceShaderDrawParametersFeatures shader_draw_features{};
            shader_draw_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETER_FEATURES;
            shader_draw_features.pNext = nullptr;
            shader_draw_features.shaderDrawParameters = VK_TRUE;

            auto device_ret = device_builder
                                  .add_pNext(&shader_draw_features)
                                  .build();
            if (device_ret)
            {
                instance->m_device = device_ret->device;
                instance->m_physical_device = selector_ret->physical_device;

                // get gpu properties
                instance->m_physical_device_properties = device_ret->physical_device.properties;

                // get queues
                auto queue_ret = device_ret->get_queue(vkb::QueueType::graphics);
                if (queue_ret)
                {
                    instance->m_graphics_queue = *queue_ret;
                    instance->m_graphics_queue_family = device_ret->get_queue_index(vkb::QueueType::graphics).value();
                }
                else
                {
                    return {InitializeError::NoSuitableGpu};
                }
            }
            else
            {
                return {InitializeError::NoSuitableGpu};
            }

            // initialize memory alloactor
            VmaAllocatorCreateInfo allocator_info{};
            allocator_info.physicalDevice = instance->m_physical_device;
            allocator_info.device = instance->m_device;
            allocator_info.instance = instance->m_instance;

            if (vmaCreateAllocator(&allocator_info, &instance->m_allocator) != VK_SUCCESS)
            {
                return {InitializeError::AllocatorCreate};
            }

            return {std::move(instance)};
        }
        return {InitializeError::FailedCreate};
    }
}