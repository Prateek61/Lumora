#pragma once

#include "Lumora/Common/Base.h"
#include "Lumora/Scripting/LuaSerializer.h"

#include <typeindex>

namespace Lumora
{
	// Base struct for asset properties
	struct AssetProps
	{
		virtual ~AssetProps() = default;

		std::string Name; // Name of the asset
		std::string Type; // Type of the asset
		std::vector<std::filesystem::path> AssetDependencies; // List of asset dependencies
		bool HotReload = false; // Enable hot reloading for this asset
		bool UpdateMetadata = false; // Update metadata if necessary
	};
}

VISITABLE_STRUCT(Lumora::AssetProps, Name, Type, AssetDependencies, HotReload, UpdateMetadata);