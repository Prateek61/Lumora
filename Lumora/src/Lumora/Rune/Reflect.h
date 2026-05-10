#pragma once

#include <utility>
#include <visit_struct/visit_struct.hpp>
#include "Lumora/Core/Defines.h"

namespace Lumora::Rune::Reflect
{
	template <typename T>
	concept Reflectable = visit_struct::traits::is_visitable<T>::value;

	template <Reflectable T, typename V>
	void ForEach(T& obj, V&& visitor)
	{
		visit_struct::for_each(obj, std::forward<V>(visitor));
	}
	template <Reflectable T, typename V>
	void ForEach(const T& obj, V&& visitor)
	{
		visit_struct::for_each(obj, std::forward<V>(visitor));
	}
}

#define LM_REFLECTABLE(...) \
	EXPAND_MACRO(VISITABLE_STRUCT(__VA_ARGS__))
