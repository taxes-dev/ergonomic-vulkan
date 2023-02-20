#include <type_traits>
#include <catch2/catch_test_macros.hpp>
#include "ergovk.hpp"

using ergovk::error_to_string;

TEST_CASE("InitializeError error_to_string covers all error values")
{
	using ergovk::InitializeError;
	using error_t = std::underlying_type_t<InitializeError>;
	for (error_t idx = 0; idx < static_cast<error_t>(InitializeError::End); idx++)
	{
		REQUIRE_FALSE(error_to_string(static_cast<InitializeError>(idx)).empty());
	}
}

TEST_CASE("ResourceError error_to_string covers all error values")
{
	using ergovk::resources::ResourceError;
	using error_t = std::underlying_type_t<ResourceError>;
	for (error_t idx = 0; idx < static_cast<error_t>(ResourceError::End); idx++)
	{
		REQUIRE_FALSE(error_to_string(static_cast<ResourceError>(idx)).empty());
	}
}