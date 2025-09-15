#pragma once

#include "Lumora/Common/Base.h"
#include "Lumora/Asset/Asset.h"
#include "Lumora/Asset/AssetProps.h"
#include "Lumora/Asset/AssetRecord.h"
#include "Lumora/Asset/AssetRegistry.h"
#include "Lumora/Asset/AssetStorage.h"

namespace Lumora
{
	class AssetManager
	{
	public:
		AssetManager(std::filesystem::path assetRoot, LuaSerializer& serializer);

		template<typename T>
			requires std::is_base_of_v<Asset, T>
		AssetHandle<T> GetAssetHandle(AssetIdT assetId);
		template<typename T>
			requires std::is_base_of_v<Asset, T>
		AssetHandle<T> GetAssetHandle(const std::string& name);
		AssetIdT RegisterAsset(const std::filesystem::path& assetPropsFile);
		AssetIdT RegisterAsset(const Ref<AssetProps>& props);
		void Load(AssetIdT assetId);
		void Reload(AssetIdT assetId);
		void Unload(AssetIdT assetId);
		void Remove(AssetIdT assetId);
		void Load(AssetRecord& record);
		void Reload(AssetRecord& record);
		void Unload(AssetRecord& record);
		bool IsValid(AssetIdT assetId) const;
		bool IsValid(const std::string& name) const;
		AssetIdT GetAssetId(const std::string& name) const;

		std::filesystem::path GetAssetRoot() const { return m_AssetRoot; }
		std::filesystem::path GetFullAssetPath(const std::filesystem::path& relativePath) const { return m_AssetRoot / relativePath; }

	private:
		AssetStorage m_Storage;
		AssetRegistry m_Registry;
		std::filesystem::path m_AssetRoot;
		LuaSerializer* m_Serializer;
	};
}

// Template Implementations
namespace Lumora
{
	template<typename T>
		requires std::is_base_of_v<Asset, T>
	AssetHandle<T> AssetManager::GetAssetHandle(AssetIdT assetId)
	{
		LM_PROFILE_FUNCTION();
		LM_CORE_ASSERT(IsValid(assetId), "Invalid Asset ID");

		return AssetHandle<T>(m_Storage.GetAssetRecord(assetId), this);
	}

	template<typename T>
		requires std::is_base_of_v<Asset, T>
	AssetHandle<T> AssetManager::GetAssetHandle(const std::string& name)
	{
		LM_PROFILE_FUNCTION();

		auto id = GetAssetId(name);
		LM_CORE_ASSERT(IsValid(id), "Invalid Asset ID");

		return AssetHandle<T>(m_Storage.GetAssetRecord(id), this);
	}
}