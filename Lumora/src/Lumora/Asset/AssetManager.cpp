#include "LMPCH.h"
#include "AssetManager.h"

#include "AssetLoader.h"

namespace Lumora
{
	AssetManager::AssetManager(std::filesystem::path assetRoot, LuaSerializer& serializer)
		: m_AssetRoot(std::move(assetRoot)), m_Serializer(&serializer)
	{
	}

	AssetIdT AssetManager::RegisterAsset(const std::filesystem::path& assetPropsFile)
	{
		LM_PROFILE_FUNCTION();

		Ref<AssetProps> props = AssetProps::DeSerialize(assetPropsFile, *m_Serializer);
		if (!props)
		{
			LM_CORE_ERROR("Error in deserializing asset props: {}", assetPropsFile.string());
			return g_INVALID_ASSET_ID;
		}

		return RegisterAsset(props);
	}

	AssetIdT AssetManager::RegisterAsset(const Ref<AssetProps>& props)
	{
		LM_PROFILE_FUNCTION();
		LM_CORE_ASSERT(props, "AssetProps is null")
		LM_CORE_ASSERT(!props->Name.empty(), "Asset name cannot be empty")
		auto existingId = m_Registry.GetAssetId(props->Name);

		if (existingId != g_INVALID_ASSET_ID)
		{
			LM_CORE_WARN("Asset with name '{}' already registered with ID {}. Returning existing ID.", props->Name, static_cast<std::string>(existingId));
			return existingId;
		}

		auto newId = UUID::Generate();
		m_Registry.RegisterAsset(newId, props);
		auto assetRecord = CreateRef<AssetRecord>(newId, props->Name, nullptr);
		m_Storage.AddAssetRecord(assetRecord);
		return newId;
	}

	void AssetManager::Load(AssetIdT assetId)
	{
		LM_PROFILE_FUNCTION();
		LM_CORE_ASSERT(IsValid(assetId), "Invalid Asset ID")

		auto record = m_Storage.GetAssetRecord(assetId);
		Load(*record);
	}

	void AssetManager::Reload(AssetIdT assetId)
	{
		LM_PROFILE_FUNCTION();
		LM_CORE_ASSERT(IsValid(assetId), "Invalid Asset ID")

		auto record = m_Storage.GetAssetRecord(assetId);
		Reload(*record);
	}

	void AssetManager::Unload(AssetIdT assetId)
	{
		LM_PROFILE_FUNCTION();
		LM_CORE_ASSERT(IsValid(assetId), "Invalid Asset ID")
		auto record = m_Storage.GetAssetRecord(assetId);
		Unload(*record);
	}

	void AssetManager::Remove(AssetIdT assetId)
	{
		LM_PROFILE_FUNCTION();
		LM_CORE_ASSERT(IsValid(assetId), "Invalid Asset ID")

		m_Storage.RemoveAssetRecord(assetId);
		m_Registry.UnregisterAsset(assetId);
	}

	void AssetManager::Load(AssetRecord& record)
	{
		LM_PROFILE_FUNCTION();

		auto props = m_Registry.GetAssetProps(record.GetAssetId());
		auto ass = AssetLoader::Load(*props);
		record.UpdateAsset(ass);
	}

	void AssetManager::Reload(AssetRecord& record)
	{
		Load(record);
	}

	void AssetManager::Unload(AssetRecord& record)
	{
		LM_PROFILE_FUNCTION();

		record.UpdateAsset(nullptr);
	}

	bool AssetManager::IsValid(AssetIdT assetId) const
	{
		LM_PROFILE_FUNCTION();

		if (assetId == g_INVALID_ASSET_ID) return false;
		return m_Storage.HasAsset(assetId);
	}

	bool AssetManager::IsValid(const std::string& name) const
	{
		LM_PROFILE_FUNCTION();

		auto id = GetAssetId(name);
		return IsValid(id);
	}

	AssetIdT AssetManager::GetAssetId(const std::string& name) const
	{
		LM_PROFILE_FUNCTION();

		return m_Registry.GetAssetId(name);
	}

}
