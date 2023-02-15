#pragma once
#include <memory>
#include <string>
#include <unordered_map>

namespace ergovk::resources
{
    /**
     * @brief Key type for resources managed by ergovk::resources::ResourceSet.
    */
	using ResourceID = std::string;

    /**
     * @brief Manages a set of resources with type \p T, referencing them by a
     * named ergovk::resources::ResourceID.
    */
	template <typename T>
	class ResourceSet
	{
	public:
        /**
         * @brief The type of pointer used by this ResourceSet.
        */
		using ResourcePtr = std::shared_ptr<T>;

        /**
         * @brief Create a new, empty ResourceSet.
        */
		ResourceSet(){};
		ResourceSet(const ResourceSet&) = delete;
		ResourceSet(ResourceSet&&) noexcept = default;
		ResourceSet& operator=(const ResourceSet&) = delete;
		ResourceSet& operator=(ResourceSet&&) noexcept = default;

        /**
         * @brief Returns whether or not this set contains a resource with the specified \p resource_id.
         * @param resource_id ergovk::resources::ResourceID
         * @returns True if a resource with the specified \p resource_id exists in this set; otherwise, false.
        */
		bool contains(const ResourceID& resource_id) const { return this->m_resources.contains(resource_id); };

        /**
         * @brief Inserts a \p resource with the specified \p resource_id into this set. If a resource already
         * exists with the specified \p resource_id, it will be replaced.
         * @param resource_id ergovk::resources::ResourceID
         * @param resource ResourcePtr
         * @returns True if \p resource was inserted, false if it was assigned to an existing resource ID.
        */
		bool insert(const ResourceID& resource_id, ResourcePtr resource)
		{
			auto [iter, result] = this->m_resources.insert_or_assign(resource_id, resource);
            return result;
		};

        /**
         * @brief Removes the resource with specified \p resource_id from this set and returns it.
         * @param resource_id
         * @returns ResourcePtr
        */
		ResourcePtr release(const ResourceID& resource_id)
		{
			assert(this->contains(resource_id));
			return std::move(this->m_resources.extract(resource_id).value());
		};

        /**
         * @brief Resets this resource set to an empty state.
        */
		void reset() { this->m_resources.clear(); };

        /**
         * @brief Gets the number of items in this resource set.
         * @returns std::size_t
        */
		std::size_t size() const { return this->m_resources.size(); };

	private:
		std::unordered_map<ResourceID, ResourcePtr> m_resources{};
	};
}