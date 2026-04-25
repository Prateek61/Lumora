#pragma once

namespace Lumora::Utils
{
	template <typename T, template <typename...> typename S, typename Enable = void>
	struct IsSpecializationOf : std::false_type {};

	template <typename... Args, template <typename...> typename S>
	struct IsSpecializationOf<S<Args...>, S> : std::true_type {};

	// Check if a template type can be instantiated with the given arguments
	template <typename T, template <typename...> typename S, typename Enable = void>
	struct IsInstantiableWith : std::false_type {};

	template <typename... Args, template <typename...> typename S>
	struct IsInstantiableWith<S<Args...>, S, std::void_t<decltype(S<Args...>{})>> : std::true_type{};

	template <typename T, template <typename...> typename S>
	constexpr bool IsSpecializationOfV = IsSpecializationOf<T, S>::value;

	template <typename T, template <typename...> typename S>
	constexpr bool IsInstantiableWithV = IsInstantiableWith<T, S>::value;
}