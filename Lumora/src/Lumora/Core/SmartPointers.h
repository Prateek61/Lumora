#pragma once

#include <memory>

namespace Lumora
{
	// Unique Pointer
	template <typename T>
	using Scope = std::unique_ptr<T>;

	template <typename T, typename... Args>
	constexpr Scope<T> CreateScope(Args&&... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	// Shared Pointer
	template <typename T>
	using Ref = std::shared_ptr<T>;

	template <typename T, typename... Args>
	constexpr Ref<T> CreateRef(Args&&... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

	// Weak Pointer
	template <typename T>
	using WeakRef = std::weak_ptr<T>;

	// Static Shared cast
	template <typename T, typename U>
	constexpr Ref<T> StaticRefCast(const Ref<U>& r) noexcept
	{
		return std::static_pointer_cast<T>(r);
	}
}