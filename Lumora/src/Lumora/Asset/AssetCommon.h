#pragma once

#include "Lumora/Common/UUID.h"

namespace Lumora
{
	using AssetIdT = UUID;
	const AssetIdT g_INVALID_ASSET_ID{};
	using AssetVersionT = uint32_t;
	constexpr AssetVersionT g_INVALID_ASSET_VERSION{ 0 };

	class Asset;
	class AssetStorage;
	class AssetRegistry;
	class AssetManager;
	struct AssetProps;

	template<typename K, typename V>
	using AssetMap = std::map<K, V>;

	template<typename T, typename PT>
		requires std::is_base_of_v<Asset, T> and std::is_base_of_v<AssetProps, PT>
	using AssetLoadFunction = std::function<Ref<T>(PT& props)>;
}
