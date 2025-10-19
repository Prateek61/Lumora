#include "LMPCH.h"

#include "AssetLoader.h"

namespace Lumora
{
	Ref<Asset> AssetLoader::Load(AssetProps& props)
	{
		// First Get the AssetTypeInfo from the AssetTypeRegistry
		auto type_info = AssetTypeRegistry::GetTypeInfo(props.Type);

		if (!type_info)
		{
			LM_CORE_ASSETS_ERROR("Failed to load asset. Asset type not registered: {}", props.Type);
			return nullptr;
		}

		return type_info->LoadFunction(props);
	}
}