#include "LMPCH.h"
#include "AssetManager.h"

#include "AssetLoader.h"

namespace Lumora
{
	AssetManager::AssetManager(const std::filesystem::path& assetRoot, LuaSerializer& serializer)
		: m_AssetRoot(assetRoot), m_Serializer(&serializer), m_AssetReloader(assetRoot, [this](AssetIdT id) {this->ReloadCallback(id); }, [this](const std::filesystem::path& path) {this->MetadataCallback(path); })
	{
		m_AssetReloader.StartWatching();
		m_AssetReloader.RunReloadThread();
	}

	std::string AssetManager::GetAssetName(AssetIdT id)
	{
		if (auto props = m_Registry.GetAssetProps(id))
		{
			return props->Name;
		}
		return "INVALID ASSET";
	}

	AssetIdT AssetManager::RegisterAsset(const std::filesystem::path& assetPropsFile)
	{
		LM_PROFILE_FUNCTION();

		auto fullPath = GetFullAssetPath(assetPropsFile);
		Ref<AssetProps> props = AssetProps::DeSerialize(fullPath, *m_Serializer);
		if (!props)
		{
			LM_CORE_ERROR("Error in deserializing asset props: {}", GetFullAssetPath(assetPropsFile).string());
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
		m_Registry.RegisterAsset(props, newId);
		auto assetRecord = CreateRef<AssetRecord>(newId, props->Name, nullptr);
		m_Storage.AddAssetRecord(assetRecord);

		LM_CORE_TRACE("Registered Asset: {} with ID {}", props->Name, static_cast<std::string>(newId));

		return newId;
	}

	bool AssetManager::Load(AssetIdT assetId)
	{
		LM_PROFILE_FUNCTION();
		LM_CORE_ASSERT(IsValid(assetId), "Invalid Asset ID")

		auto record = m_Storage.GetAssetRecord(assetId);
		if (!record)	return false;
		return Load(*record);
	}

	bool AssetManager::Unload(AssetIdT assetId)
	{
		LM_PROFILE_FUNCTION();
		LM_CORE_ASSERT(IsValid(assetId), "Invalid Asset ID")

		auto record = m_Storage.GetAssetRecord(assetId);
		if (!record)	return false;
		return Unload(*record);
	}

	bool AssetManager::Remove(AssetIdT assetId)
	{
		LM_PROFILE_FUNCTION();
		LM_CORE_ASSERT(IsValid(assetId), "Invalid Asset ID")

		m_Storage.RemoveAssetRecord(assetId);
		m_Registry.UnregisterAsset(assetId);
		return true;
	}

	bool AssetManager::Load(AssetRecord& record)
	{
		LM_PROFILE_FUNCTION();

		auto props = m_Registry.GetAssetProps(record.GetAssetId());
		if (!props)
		{
			return false;
		}

		auto ass = AssetLoader::Load(*props);
		if (!ass)
		{
			return false;
		}

		record.UpdateAsset(ass);
		return true;
	}

	bool AssetManager::Unload(AssetRecord& record)
	{
		LM_PROFILE_FUNCTION();

		record.UpdateAsset(nullptr);
		return true;
	}

	bool AssetManager::Remove(AssetRecord& record)
	{
		return Remove(record.GetAssetId());
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

	void AssetManager::ReloadCallback(AssetIdT id)
	{
		LM_PROFILE_FUNCTION();

		LM_CORE_TRACE("Reloading Asset: {}", GetAssetName(id));
	}

	void AssetManager::MetadataCallback(const std::filesystem::path& path)
	{
		LM_PROFILE_FUNCTION();

		LM_CORE_TRACE("Meta file changed: {}", path.string());
	}

}
