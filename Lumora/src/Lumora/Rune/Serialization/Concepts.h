#pragma once

#include "Lumora/Utilities/TypeMeta.h"
#include <type_traits>
#include <vector>
#include <map>
#include <unordered_map>
#include <sol/types.hpp>
#include <string>

namespace Lumora::Rune::Serialization
{
	template <typename T>
	concept Arithmetic = std::is_arithmetic_v<T> && !std::is_same_v<T, bool>;

	template <typename T>
	concept StringLike = std::is_same_v <T, std::string>;

	template <typename T>
	concept VectorLike = Utils::IsSpecializationOfV<T, std::vector> && requires
	{
		typename T::value_type;
	};

	template <typename T>
	concept MapLike = (Utils::IsSpecializationOfV<T, std::map> || Utils::IsSpecializationOfV<T, std::unordered_map>) &&
		requires
		{
			typename T::key_type;
			typename T::mapped_type;
		};

	template <typename T>
	concept KeyType = Arithmetic<T> || StringLike<T>;

	template <typename T>
	struct IsSolPrimitive
	{
		static constexpr bool Value = sol::is_lua_primitive_v<T>;
	};

	template <typename T>
	struct IsSolContainer
	{
		static constexpr bool Value = sol::is_container_v<T>;
	};

	template <typename T>
	struct IsSolSupported
	{
		static constexpr bool Value = IsSolPrimitive<T>::Value;
	};

	template <MapLike M>
	struct IsSolSupported<M>
	{
		static constexpr bool Value = IsSolSupported<typename M::key_type>::Value && KeyType<typename M::key_type> && IsSolSupported<typename M::mapped_type>::Value;
	};

	template <VectorLike V>
	struct IsSolSupported<V>
	{
		static constexpr bool Value =  IsSolContainer<V>::Value && IsSolSupported<typename V::value_type>::Value;
	};

	template <typename T>
	concept SolSupportedType = IsSolSupported<std::decay_t<T>>::Value;
}
