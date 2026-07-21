#pragma once

#include <Lumora/Rune/Reflect.h>
#include <Lumora/Aether/Entity.h>

#include <string>
#include <cstdint>
#include <string_view>
#include <typeindex>
#include <type_traits>

namespace Lumora::Atlas
{
	struct AssetMetaFile
	{
		std::string Name;
		std::string Type;
	};

	struct AssetId
	{
		uint64_t Id;

		constexpr bool IsValid() const { return Id != 0; }
		constexpr explicit operator bool() const { return IsValid(); }
		constexpr bool operator==(const AssetId& other) const { return Id == other.Id; }

		static AssetId Generate(std::string_view name);
		static constexpr AssetId Invalid() { return AssetId{0}; }
	};

	struct AssetMeta
	{
		AssetId Id;
		std::string Name;
		std::string Type;
		bool HotReload = true;
		bool IsDefault = false;
	};

	template <typename T>
	struct AssetHandle
	{
		AssetId Id;
		Aether::Entity Entity;

		const T& Get() const { return Entity.Get<T>(); }
		T& Get() { return Entity.GetMut<T>(); }
		const T* TryGet() const { return Entity.TryGet<T>(); }
		T* TryGet() { return Entity.TryGetMut<T>(); }

		/// Runs `fn(T&)` if the asset is loaded, does nothing if it isn't.
		template<typename F>
		void operator &&(F&& fn);

		bool IsValid() const { return Id.IsValid() && Entity.IsValid(); }
		bool IsLoaded() const { return IsValid() && Entity.Has<T>(); }

		constexpr bool operator==(const AssetHandle& other) const = default;
	};

	struct UntypedAssetHandle
	{
		AssetId Id;
		std::type_index Type{typeid(void)};
		Aether::Entity Entity;

		constexpr bool IsValid() const { return Id.IsValid(); }

		template <typename T>
		AssetHandle<T> As() const
		{
			if (Type != typeid(T))
				return AssetHandle<T>{AssetId::Invalid(), {}};
			return AssetHandle<T>{Id, Entity};
		}
	};

	struct AssetTag{};
}

namespace Lumora::Atlas
{
	template<typename T>
	template<typename F>
	void AssetHandle<T>::operator&&(F&& fn)
	{
		if (!IsValid())
			return;
		
		T* ptr = Entity.TryGetMut<T>();
		if (ptr)
		{
			std::forward<F>(fn)(*ptr);
		}
	}
}

namespace std
{
	template<>
	struct hash<Lumora::Atlas::AssetId>
	{
		size_t operator()(const Lumora::Atlas::AssetId& assetId) const noexcept
		{
			return std::hash<uint64_t>()(assetId.Id);
		}
	};
}

LM_REFLECTABLE(Lumora::Atlas::AssetMetaFile, Name, Type);
LM_REFLECTABLE(Lumora::Atlas::AssetMeta, Id, Name, Type, HotReload, IsDefault);
