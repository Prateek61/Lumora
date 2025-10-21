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
	Lumora::AssetMap<std::string, Lumora::Ref<Lumora::AssetTypeInfo>>& GetNameToTypeInfoMap()
	{
		static Lumora::AssetMap<std::string, Lumora::Ref<Lumora::AssetTypeInfo>> map;
		return map;
	}
}

namespace Lumora
{
	void AssetTypeRegistry::RegisterType(Ref<AssetTypeInfo> typeInfo)
	{
		DEBUG_ONLY
		(
			// Ensure the type isn't already registered
			auto itType = GetTypeToNameMap().find(typeInfo->AssetType);
			LM_CORE_ASSERT(itType == GetTypeToNameMap().end(), "Asset type already registered: (" + typeInfo->Name + ")");
			auto itName = GetNameToTypeInfoMap().find(typeInfo->Name);
			LM_CORE_ASSERT(itName == GetNameToTypeInfoMap().end(), "Duplicate Asset Name or already registered: (" + typeInfo->
				Name + ")");
		)

		GetTypeToNameMap()[typeInfo->AssetType] = typeInfo->Name;
		GetNameToTypeInfoMap()[typeInfo->Name] = std::move(typeInfo);
	}

	std::string AssetTypeRegistry::GetName(std::type_index assetType)
	{
		LM_PROFILE_FUNCTION();

		auto it = GetTypeToNameMap().find(assetType);
		if (it == GetTypeToNameMap().end())
		{
			LM_CORE_ASSETS_ERROR("Asset type not registered: {}", assetType.name());
			return "";
		}

		return it->second;
	}

	Ref<AssetTypeInfo> AssetTypeRegistry::GetTypeInfo(const std::string& type)
	{
		LM_PROFILE_FUNCTION();
		auto it = GetNameToTypeInfoMap().find(type);

		if (it == GetNameToTypeInfoMap().end())
		{
			LM_CORE_ASSETS_ERROR("Asset type not registered, or rewritten: {}", type);
			return nullptr;
		}

		return it->second;
	}

	Ref<AssetTypeInfo> AssetTypeRegistry::GetTypeInfo(std::type_index assetType)
	{
		LM_PROFILE_FUNCTION();
		auto name = GetName(assetType);
		return GetTypeInfo(name);
	}
}
