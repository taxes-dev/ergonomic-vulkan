#pragma once
#include <concepts>
#include <functional>
#include <string_view>
#include <variant>
#include <vector>
#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>

#include "ergovk_structs.hpp"

#ifndef NDEBUG
#define ERGOVK_DEBUG
#endif

namespace vkb
{
	class InstanceBuilder;
}

namespace ergovk
{
	/// @brief Errors generated by VulkanInstanceBuilder::build()
	enum class InitializeError
	{
		/// @brief Could not create an allocator (VMA)
		AllocatorCreate,
		/// @brief Initial creation of VkInstance failed
		FailedCreate,
		/// @brief Cannot find a GPU that matches the requirements for rendering
		NoSuitableGpu,
		/// @brief The surface creation callback did not return a valid surface
		SurfaceCreate,
		SwapchainCreate,
	};

	/// @brief Errors generated by Swapchain::create()
	enum class SwapchainCreateError
	{
		/// @brief Could not allocate the depth image buffer
		DepthImageAllocation,
		/// @brief Could not allocate the depth image view
		DepthImageViewAllocation,
		/// @brief Initial creation of VkSwapchainKHR failed
		FailedCreate,
	};

	/** @brief Convert an error enum to a human readable string
     * @param error ergovk::InitializeError value
     * @return std::string_view
     */
	std::string_view error_to_string(InitializeError error);

	/** @brief Convert an error enum to a human readable string
     * @param error ergovk::SwapchainCreateError value
     * @return std::string_view
     */
	std::string_view error_to_string(SwapchainCreateError error);

	/** @brief Constraint to validate if \p T is an enumeration
     * @tparam T a type to check
     * @return true if \p T is an enumeration, otherwise false
     */
	template <typename T>
	concept IsEnum = std::is_enum<T>::value;

	/** @brief Used for returning results from a function that can fail.
     * @tparam TOk is the type of result when successful
     * @tparam TError is an enumeration that represents an error
     */
	template <typename TOk, IsEnum TError>
	using Result = std::variant<TOk, TError>;

	/** @brief Checks a \p result for a valid result
     * @param result ergovk::Result<TOk, TError>
     * @return true if \p result contains a valid result, otherwise false
     */
	template <typename TOk, IsEnum TError>
	inline bool is_ok(const Result<TOk, TError>& result)
	{
		return std::holds_alternative<TOk>(result);
	}

	/** @brief Checks \p result for an error
     * @param result ergovk::Result<TOk, TError>
     * @return true if \p result contains an error enum, otherwise false
     */
	template <typename TOk, IsEnum TError>
	inline bool is_error(const Result<TOk, TError>& result)
	{
		return std::holds_alternative<TError>(result);
	}

	/** @brief Gets the error contained in \p result
     * @param result ergovk::Result<TOk, TError>
     * @return a \p TError value
     */
	template <typename TOk, IsEnum TError>
	const TError& get_error(const Result<TOk, TError>& result)
	{
		assert(is_error(result));
		return std::get<TError>(result);
	}

	/** @brief Gets the value contained in \p result
     * @param result ergovk::Result<TOk, TError>
     * @return a \p TOk value
     * @note \p TOk must be copyable
     */
	template <typename TOk, IsEnum TError>
		requires std::is_copy_assignable<TOk>::value
	TOk get_value(const Result<TOk, TError>& result)
	{
		assert(is_ok(result));
		return std::get<TOk>(result);
	}

	/** @brief Moves the value out of \p result
     * @param result ergovk::Result<TOk, TError>
     * @return a \p TOk value
     * @note \p TOk must be movable
     */
	template <typename TOk, IsEnum TError>
		requires std::is_move_assignable<TOk>::value
	[[nodiscard]] TOk&& unwrap(Result<TOk, TError>& result)
	{
		assert(is_ok(result));
		return std::get<TOk>(std::move(result));
	}

	/**
     * @brief Contains a reference to an image buffer allocated by VMA.
    */
	class VkImageHandle
	{
	public:
		/**
     * @brief Create a new, empty ergovk::VkImageHandle.
    */
		VkImageHandle() noexcept {};
		/**
         * @brief Create a new ergovk::VkImageHandle.
         * @param allocator VmaAllocator
         * @param device VkDevice
         * @param image VkImage
         * @param allocation VmaAllocation
        */
		VkImageHandle(VmaAllocator allocator, VkDevice device, VkImage image, VmaAllocation allocation)
			: m_allocator{ allocator }, m_device{ device }, image{ image }, allocation{ allocation }
		{
			assert(allocator != VK_NULL_HANDLE);
			assert(device != VK_NULL_HANDLE);
			assert(image != VK_NULL_HANDLE);
			assert(allocation != VK_NULL_HANDLE);
		};
		~VkImageHandle() { this->destroy(); }
		VkImageHandle(const VkImageHandle&) = delete;
		VkImageHandle(VkImageHandle&& other) noexcept { *this = std::move(other); };
		VkImageHandle& operator=(const VkImageHandle&) = delete;
		VkImageHandle& operator=(VkImageHandle&& other) noexcept
		{
			this->m_allocator = other.m_allocator;
			this->m_device = other.m_device;
			this->allocation = other.allocation;
			this->image = other.image;
			other.allocation = VK_NULL_HANDLE;
			other.image = VK_NULL_HANDLE;
			return *this;
		};

		/**
         * @brief Allocates a new image buffer and returns the handle.
         * @param allocator VmaAllocator
         * @param device VkDevice
         * @param create_info VkImageCreateInfo
         * @param allocation_create_info VmaAllocationCreateInfo
         * @return ergovk::VkImageHandle on success, otherwise VkResult
        */
		static Result<VkImageHandle, VkResult> create(VmaAllocator allocator, VkDevice device,
			VkImageCreateInfo create_info, VmaAllocationCreateInfo allocation_create_info);

		/**
         * @brief Explicitly destroys all of the resources managed by this instance.
        */
		void destroy();

		/**
         * @brief Handle to the image buffer.
        */
		VkImage image{ VK_NULL_HANDLE };

		/**
         * @brief Handle to the image buffer's allocation.
        */
		VmaAllocation allocation{ VK_NULL_HANDLE };

	private:
		VmaAllocator m_allocator{ VK_NULL_HANDLE };
		VkDevice m_device{ VK_NULL_HANDLE };
	};

	/**
     * @brief Contains a reference to an image view.
    */
	class VkImageViewHandle
	{
	public:
		/**
     * @brief Create a new, empty ergovk::VkImageViewHandle.
    */
		VkImageViewHandle() noexcept {};
		/**
         * Create a new ergovk::VkImageViewHandle.
         * @param device VkDevice
         * @param image_view VkImageView
        */
		VkImageViewHandle(VkDevice device, VkImageView image_view) : m_device{ device }, image_view{ image_view }
		{
			assert(device != VK_NULL_HANDLE);
			assert(image_view != VK_NULL_HANDLE);
		};
		~VkImageViewHandle() { this->destroy(); };
		VkImageViewHandle(const VkImageViewHandle&) = delete;
		VkImageViewHandle(VkImageViewHandle&& other) noexcept { *this = std::move(other); }
		VkImageViewHandle& operator=(const VkImageViewHandle&) = delete;
		VkImageViewHandle& operator=(VkImageViewHandle&& other) noexcept
		{
			this->m_device = other.m_device;
			this->image_view = other.image_view;
			other.image_view = VK_NULL_HANDLE;
			return *this;
		}

		/**
         * @brief Allocates a new image view and returns the handle.
         * @param device VkDevice
         * @param create_info VkImageViewCreateInfo
         * @return ergovk::VkImageViewHandle on success, otherwise VkResult
        */
		static Result<VkImageViewHandle, VkResult> create(VkDevice device, VkImageViewCreateInfo create_info);

		/**
         * @brief Explicitly destroys all of the resources managed by this instance.
        */
		void destroy();

		/**
         * @brief Handle to the image view.
        */
		VkImageView image_view{ VK_NULL_HANDLE };

	private:
		VkDevice m_device{ VK_NULL_HANDLE };
	};

	/**
     * @breif A swapchain of image buffers used for rendering.
    */
	class Swapchain
	{
	public:
		/**
         * @brief Creates a new, empty ergovk::Swapchain.
        */
		Swapchain() noexcept {};
		~Swapchain() { this->destroy(); };
		Swapchain(const Swapchain&) = delete;
		Swapchain(Swapchain&& other) noexcept { *this = std::move(other); };
		Swapchain& operator=(const Swapchain&) = delete;
		Swapchain& operator=(Swapchain&& other) noexcept
		{
			this->m_device = other.m_device;
			this->m_swapchain = other.m_swapchain;
			other.m_swapchain = VK_NULL_HANDLE;
			this->m_images = std::move(other.m_images);
			this->m_image_views = std::move(other.m_image_views);
			this->m_image_format = other.m_image_format;
			this->m_depth_format = other.m_depth_format;
			this->m_depth_image = std::move(other.m_depth_image);
			this->m_depth_image_view = std::move(other.m_depth_image_view);
			return *this;
		};

		/**
         * @brief Allocates a new swapchain with images and views as well as a depth buffer.
         * @param allocator VmaAllocator
         * @param physical_device VkPhysicalDevice
         * @param device VkDevice
         * @param surface VkSurfaceKHR
         * @param extent VkExtent2D with the dimensions of \p surface
         * @return ergovk::Swapchain on success, otherwise VkResult
        */
		static Result<Swapchain, SwapchainCreateError> create(VmaAllocator allocator, VkPhysicalDevice physical_device,
			VkDevice device, VkSurfaceKHR surface, VkExtent2D extent);

		/**
         * @brief Explicitly destroys all of the resources managed by this instance.
        */
		void destroy();

	private:
		VkDevice m_device{ VK_NULL_HANDLE };
		VkSwapchainKHR m_swapchain{ VK_NULL_HANDLE };
		// m_images doesn't use a wrapper b/c it will be cleaned up when m_swapchain is destroyed
		std::vector<VkImage> m_images{};
		std::vector<VkImageViewHandle> m_image_views{};
		VkFormat m_image_format{ VkFormat::VK_FORMAT_UNDEFINED };
		VkFormat m_depth_format{ VkFormat::VK_FORMAT_UNDEFINED };
		VkImageHandle m_depth_image{};
		VkImageViewHandle m_depth_image_view{};
	};

	/**
     * @brief Represents the currently running Vulkan context and manages its resources. Must be created using
     * an ergovk::VulkanInstanceBuilder.
     * @see ergovk::VulkanInstanceBuilder
     */
	class VulkanInstance
	{
		friend class VulkanInstanceBuilder;

	public:
		VulkanInstance(const VulkanInstance&) = delete;
		VulkanInstance(VulkanInstance&& other) noexcept { *this = std::move(other); };
		~VulkanInstance() { this->destroy(); }
		VulkanInstance& operator=(const VulkanInstance&) = delete;
		VulkanInstance& operator=(VulkanInstance&& other) noexcept
		{
			this->instance = other.instance;
			other.instance = VK_NULL_HANDLE;
			this->device = other.device;
			other.device = VK_NULL_HANDLE;
			this->m_debug_messenger = other.m_debug_messenger;
			other.m_debug_messenger = VK_NULL_HANDLE;
			this->m_surface = other.m_surface;
			other.m_surface = VK_NULL_HANDLE;
			this->m_physical_device = other.m_physical_device;
			this->m_physical_device_properties = other.m_physical_device_properties;
			this->m_graphics_queue = other.m_graphics_queue;
			this->m_graphics_queue_family = other.m_graphics_queue_family;
			this->m_allocator = other.m_allocator;
			other.m_allocator = VK_NULL_HANDLE;
			this->m_swapchain = std::move(other.m_swapchain);
			return *this;
		};

		/**
         * @brief Explicitly destroys all of the resources managed by this instance.
        */
		void destroy();


		/** @brief Gets the minimum uniform buffer offset alignment, needed for properly aligning data in descriptors
         * @return VkDeviceSize
         */
		VkDeviceSize get_min_uniform_buffer_offset_alignment() const
		{
			if (this->m_physical_device)
			{
				return this->m_physical_device_properties.limits.minUniformBufferOffsetAlignment;
			}
			return 0;
		}

		/**
         * @brief Handle the Vulkan instance.
        */
		VkInstance instance{ VK_NULL_HANDLE };

		/**
         * @brief Handle to the selected device.
        */
		VkDevice device{ VK_NULL_HANDLE };

	private:
		VulkanInstance() noexcept {};

		VkDebugUtilsMessengerEXT m_debug_messenger{ VK_NULL_HANDLE };
		VkSurfaceKHR m_surface{ VK_NULL_HANDLE };
		VkPhysicalDevice m_physical_device{ VK_NULL_HANDLE };
		VkPhysicalDeviceProperties m_physical_device_properties{};
		VkQueue m_graphics_queue{ VK_NULL_HANDLE };
		std::uint32_t m_graphics_queue_family{ 0 };
		VmaAllocator m_allocator{ VK_NULL_HANDLE };
		Swapchain m_swapchain{};
	};

	/** @brief Builds a valid ergovk::VulkanInstance based on provided parameters.
     * @see ergovk::VulkanInstance
     */
	class VulkanInstanceBuilder
	{
	public:
		/// @brief Callback signature used with \p set_create_surface_callback()
		using CreateSurfaceCallback = std::function<VkSurfaceKHR(VkInstance)>;

		/// @brief Creates a new empty ergovk::VulkanInstanceBuilder
		VulkanInstanceBuilder() noexcept;
		~VulkanInstanceBuilder();
		VulkanInstanceBuilder(const VulkanInstanceBuilder&) = delete;
		VulkanInstanceBuilder(VulkanInstanceBuilder&&) noexcept = default;
		VulkanInstanceBuilder& operator=(const VulkanInstanceBuilder&) = delete;
		VulkanInstanceBuilder& operator=(VulkanInstanceBuilder&&) noexcept = default;

		/** @brief REQUIRED: Used when the builder needs to create a VkSurfaceKHR for rendering
         * @param callback ergovk::VulkanInstanceBuilder::CreateSurfaceCallback
         * @return this
         * @note \p callback is not retained after the call to \p build()
         */
		VulkanInstanceBuilder& set_create_surface_callback(CreateSurfaceCallback&& callback);

		/** @brief OPTIONAL: In debug builds, use this to create a custom messenger for validation layers
         * @param callback PFN_vkDebugUtilsMessengerCallbackEXT
         * @return this
         * @note \p callback must be addressable for the lifetime of the ergovk::VulkanInstance
         */
		VulkanInstanceBuilder& set_custom_debug_callback(PFN_vkDebugUtilsMessengerCallbackEXT callback);

		/** @brief REQUIRED: Set the extents of the drawing surface.
         * @param extent VkExtent2D
         * @return this
         * @note \p extent should match the dimensions of the surface created by \p set_create_surface_callback()
        */
		VulkanInstanceBuilder& set_draw_extent(VkExtent2D extent);

		/** @brief Attempts to create an ergovk::VulkanInstance from the parameters stored in this builder
         * @return Result<ergovk::VulkanInstance, ergovk::InitializeError>
         */
		[[nodiscard]] Result<VulkanInstance, InitializeError> build() const;

	private:
		vkb::InstanceBuilder* m_builder{ nullptr };
		CreateSurfaceCallback m_create_surface_callback{};
		VkExtent2D m_extent{ 1, 1 };
	};

}