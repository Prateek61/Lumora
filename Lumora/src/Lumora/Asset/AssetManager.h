#pragma once

#include "Lumora/Common/Base.h"
#include "Lumora/Asset/Asset.h"
#include "Lumora/Asset/AssetProps.h"
#include "Lumora/Asset/AssetRecord.h"
#include "Lumora/Asset/AssetRegistry.h"
#include "Lumora/Asset/AssetStorage.h"
#include "Lumora/Asset/AssetReloader.h"

namespace Lumora
{
	class AssetManager
	{
	public:
		AssetManager(const std::filesystem::path& assetRoot, LuaSerializer& serializer);

		template<typename T>
			requires std::is_base_of_v<Asset, T>
		AssetHandle<T> GetAssetHandle(AssetIdT assetId);
		template<typename T>
			requires std::is_base_of_v<Asset, T>
		AssetHandle<T> GetAssetHandle(const std::string& name);
		std::string GetAssetName(AssetIdT id);

		AssetIdT RegisterAsset(const std::filesystem::path& assetPropsFile);
		AssetIdT RegisterAsset(const Ref<AssetProps>& props);

		bool Load(AssetIdT assetId);
		bool Unload(AssetIdT assetId);
		bool Remove(AssetIdT assetId);

		bool Load(AssetRecord& record);
		bool Unload(AssetRecord& record);
		bool Remove(AssetRecord& record);

		bool IsValid(AssetIdT assetId) const;
		bool IsValid(const std::string& name) const;

		AssetIdT GetAssetId(const std::string& name) const;

		std::filesystem::path GetAssetRoot() const { return m_AssetRoot; }
		std::filesystem::path GetFullAssetPath(const std::filesystem::path& relativePath) const { return m_AssetRoot / relativePath; }
		static bool IsMetaFile(const std::filesystem::path& path) { return path.extension() == ".lua" && path.stem().extension() == ".meta"; }

	private:
		AssetStorage m_Storage;
		AssetRegistry m_Registry;
		std::filesystem::path m_AssetRoot;
		LuaSerializer* m_Serializer;
		AssetReloader m_AssetReloader;

	private:
		void ReloadCallback(AssetIdT id);
		void MetadataCallback(const std::filesystem::path& path);
		void ScanFolder(const std::filesystem::path& path);
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
		LM_CORE_ASSERT(IsValid(assetId), "Invalid Asset ID")

		return AssetHandle<T>(m_Storage.GetAssetRecord(assetId), this);
	}

	template<typename T>
		requires std::is_base_of_v<Asset, T>
	AssetHandle<T> AssetManager::GetAssetHandle(const std::string& name)
	{
		LM_PROFILE_FUNCTION();

		auto id = GetAssetId(name);
		LM_CORE_ASSERT(IsValid(id), "Invalid Asset ID")

		auto record = m_Storage.GetAssetRecord(id);
		LM_CORE_ASSERT(record, "Asset Record not found");
		return AssetHandle<T>(record, this);
	}
}