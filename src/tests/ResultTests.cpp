#include <catch2/catch_test_macros.hpp>
#include "ergovk.hpp"

using ergovk::Result;
using ergovk::is_ok;
using ergovk::is_error;
using ergovk::get_value;
using ergovk::get_error;
using ergovk::unwrap;

struct OkVal
{
	int value{ 0 };
};
enum class ErrorVal
{
	Error,
};

TEST_CASE("Ok Result can be retrieved")
{
	Result<OkVal, ErrorVal> result{ OkVal{ .value = 1 } };

	REQUIRE(is_ok(result));
	REQUIRE_FALSE(is_error(result));
	REQUIRE(get_value(result).value == 1);
}

TEST_CASE("Error Result can be retrieved")
{
	Result<OkVal, ErrorVal> result{ ErrorVal::Error };

	REQUIRE(is_error(result));
	REQUIRE_FALSE(is_ok(result));
	REQUIRE(get_error(result) == ErrorVal::Error);
}

TEST_CASE("Ok Result can be unwrapped")
{
	Result<OkVal, ErrorVal> result{ OkVal{ .value = 2 } };

	REQUIRE(is_ok(result));
	auto ok = unwrap(result);
	REQUIRE(ok.value == 2);
}