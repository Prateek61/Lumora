#include "LMPCH.h"

#include "Lumora/Asset/AssetCommon.h"
#include "AssetTypeRegistry.h"

namespace
{
	Lumora::AssetMap<std::type_index, std::string>& GetTypeToNameMap()
	{
		static Lumora::AssetMap<std::type_index, std::string> map;
		return map;
	}
	Lumora::AssetMap<std::string, Lumora::AssetTypeInfo>& GetNameToTypeInfoMap()
	{
		static Lumora::AssetMap<std::string, Lumora::AssetTypeInfo> map;
		return map;
	}
}

namespace Lumora
{
	void AssetTypeRegistry::RegisterType(AssetTypeInfo typeInfo)
	{
		DEBUG_ONLY
		(
			// Ensure the type isn't already registered
			auto itType = GetTypeToNameMap().find(typeInfo.AssetType);
			LM_CORE_ASSERT(itType == GetTypeToNameMap().end(), "Asset type already registered");
			auto itName = GetNameToTypeInfoMap().find(typeInfo.Name);
			LM_CORE_ASSERT(itName == GetNameToTypeInfoMap().end(), "Duplicate Asset Name or already registered: (" + typeInfo.
				Name + ")");
		)

		GetTypeToNameMap()[typeInfo.AssetType] = typeInfo.Name;
		GetNameToTypeInfoMap()[typeInfo.Name] = std::move(typeInfo);
	}

	std::string AssetTypeRegistry::GetName(std::type_index assetType)
	{
		LM_PROFILE_FUNCTION();
		auto it = GetTypeToNameMap().find(assetType);
		LM_CORE_ASSERT(it != GetTypeToNameMap().end(), "Asset type not registered")
		return it->second;
	}

	AssetTypeInfo& AssetTypeRegistry::GetTypeInfo(const std::string& type)
	{
		LM_PROFILE_FUNCTION();
		auto it = GetNameToTypeInfoMap().find(type);
		LM_CORE_ASSERT(it != GetNameToTypeInfoMap().end(), "Asset type not registered, or rewritten");
		return it->second;
	}

	AssetTypeInfo& AssetTypeRegistry::GetTypeInfo(std::type_index assetType)
	{
		LM_PROFILE_FUNCTION();
		auto name = GetName(assetType);
		return GetTypeInfo(name);
	}
}
