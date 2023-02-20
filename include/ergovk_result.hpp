#pragma once
#include <concepts>
#include <variant>

namespace ergovk
{
	/**
	 * @brief Constraint to validate if \p T is an enumeration
     * @tparam T a type to check
     * @return true if \p T is an enumeration, otherwise false
     */
	template <typename T>
	concept IsEnum = std::is_enum<T>::value;

	/**
	 * @brief Used for returning results from a function that can fail.
     * @tparam TOk is the type of result when successful
     * @tparam TError is an enumeration that represents an error
     */
	template <typename TOk, IsEnum TError>
	using Result = std::variant<TOk, TError>;

	/**
	 * @brief Checks a \p result for a valid result.
     * @param result ergovk::Result<TOk, TError>
     * @return true if \p result contains a valid result, otherwise false
     */
	template <typename TOk, IsEnum TError>
	inline bool is_ok(const Result<TOk, TError>& result)
	{
		return std::holds_alternative<TOk>(result);
	}

	/**
	 * @brief Checks \p result for an error.
     * @param result ergovk::Result<TOk, TError>
     * @return true if \p result contains an error enum, otherwise false
     */
	template <typename TOk, IsEnum TError>
	inline bool is_error(const Result<TOk, TError>& result)
	{
		return std::holds_alternative<TError>(result);
	}

	/**
	 * @brief Gets the error contained in \p result.
     * @param result ergovk::Result<TOk, TError>
     * @return a \p TError value
     */
	template <typename TOk, IsEnum TError>
	const TError& get_error(const Result<TOk, TError>& result)
	{
		assert(is_error(result));
		return std::get<TError>(result);
	}

	/**
	 * @brief Gets the value contained in \p result.
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

	/**
	 * @brief Moves the value out of \p result.
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
}