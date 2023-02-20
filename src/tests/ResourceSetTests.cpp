#include <catch2/catch_test_macros.hpp>
#include "ergovk.hpp"

using namespace ergovk::resources;
using ergovk::is_ok;
using ergovk::is_error;
using ergovk::get_value;
using ergovk::get_error;

class TestResource
{
public:
	TestResource(){};
	~TestResource(){};
};

TEST_CASE("ResourceSet insert increases size, re-assign does not")
{
	ResourceSet<TestResource> set{};

	// inserting
	REQUIRE(set.size() == 0);
	REQUIRE(set.insert("1", std::make_shared<TestResource>()));
	REQUIRE(set.size() == 1);
	REQUIRE(set.insert("2", std::make_shared<TestResource>()));
	REQUIRE(set.size() == 2);

	// overwriting
	REQUIRE_FALSE(set.insert("1", std::make_shared<TestResource>()));
	REQUIRE(set.size() == 2);
}

TEST_CASE("ResourceSet re-assign overwrites previous value")
{
	ResourceSet<TestResource> set{};

	auto resource1 = std::make_shared<TestResource>();
	auto resource2 = std::make_shared<TestResource>();

	REQUIRE(set.insert("1", resource1));
	REQUIRE(set.size() == 1);
	auto resource1b = set.get("1");
	REQUIRE(is_ok(resource1b));
	REQUIRE(get_value(resource1b) == resource1);

	REQUIRE_FALSE(set.insert("1", resource2));
	REQUIRE(set.size() == 1);
	auto resource2b = set.get("1");
	REQUIRE(is_ok(resource2b));
	REQUIRE(get_value(resource2b) == resource2);
}

TEST_CASE("ResourceSet retrieve by ID returns resource or error")
{
	ResourceSet<TestResource> set{};

	auto resource1 = std::make_shared<TestResource>();
	auto resource2 = std::make_shared<TestResource>();

	REQUIRE(set.insert("1", resource1));
	REQUIRE(set.insert("2", resource2));
	REQUIRE(set.size() == 2);
	REQUIRE(set.contains("1"));
	REQUIRE(set.contains("2"));
	REQUIRE_FALSE(set.contains("3"));

	auto resource1b = set.get("1");
	REQUIRE(is_ok(resource1b));
	REQUIRE(get_value(resource1b) == resource1);

	auto resource2b = set.get("2");
	REQUIRE(is_ok(resource2b));
	REQUIRE(get_value(resource2b) == resource2);
	REQUIRE(get_value(resource1b) != get_value(resource2b));

	auto resource3b = set.get("3");
	REQUIRE(is_error(resource3b));
	REQUIRE(get_error(resource3b) == ResourceError::ResourceNotPresent);
}

TEST_CASE("ResourceSet releases a specific resource")
{
	ResourceSet<TestResource> set{};

	auto resource1 = std::make_shared<TestResource>();
	auto resource2 = std::make_shared<TestResource>();

	REQUIRE(set.insert("1", resource1));
	REQUIRE(set.insert("2", resource2));
	REQUIRE(set.size() == 2);

	auto resource1b = set.release("1");
	REQUIRE(is_ok(resource1b));
	REQUIRE(get_value(resource1b) == resource1);
	REQUIRE(set.size() == 1);
	auto resource1c = set.get("1");
	REQUIRE(is_error(resource1c));
	REQUIRE(get_error(resource1c) == ResourceError::ResourceNotPresent);
}

TEST_CASE("ResourceSet can be reset")
{
	ResourceSet<TestResource> set{};

	REQUIRE(set.insert("1", std::make_shared<TestResource>()));
	REQUIRE(set.insert("2", std::make_shared<TestResource>()));
	REQUIRE(set.insert("3", std::make_shared<TestResource>()));
	REQUIRE(set.size() == 3);
	set.reset();
	REQUIRE(set.size() == 0);
}