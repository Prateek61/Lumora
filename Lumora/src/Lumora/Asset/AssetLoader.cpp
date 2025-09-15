#include "LMPCH.h"

#include "AssetLoader.h"

namespace Lumora
{
	Ref<Asset> AssetLoader::Load(AssetProps& props)
	{
		// First Get the AssetTypeInfo from the AssetTypeRegistry
		auto type_info = AssetTypeRegistry::GetTypeInfo(props.Type);
		return type_info.LoadFunction(props);
	}
}