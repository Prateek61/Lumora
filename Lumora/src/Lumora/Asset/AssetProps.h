#pragma once

#include "Lumora/Common/Base.h"
#include "Lumora/Scripting/LuaSerializer.h"
#include "Lumora/Asset/AssetCommon.h"

#include <typeindex>

namespace Lumora
{
	class LuaSerializer;
	

	// Base struct for asset properties
	struct AssetProps
	{
		virtual ~AssetProps() = default;

		std::string Name; // Name of the asset
		std::string Type; // Type of the asset
		std::vector<std::filesystem::path> AssetDependencies; // List of asset dependencies
		bool HotReload = false; // Enable hot reloading for this asset
		bool UpdateMetadata = false; // Update metadata if necessary
		AssetIdT AssetId = g_INVALID_ASSET_ID; // Asset ID, assigned by the AssetRegistry

		std::string ToString() const;
		static Ref<AssetProps> DeSerialize(const std::filesystem::path& propsPath, LuaSerializer& serializer);
	};
}

VISITABLE_STRUCT(Lumora::AssetProps, Name, Type, AssetDependencies, HotReload, UpdateMetadata);

#define LM_VISITABLE_ASSET_PROPS(PropsType, ...) \
	VISITABLE_STRUCT(PropsType, Name, Type, AssetDependencies, HotReload, UpdateMetadata __VA_ARGS__);