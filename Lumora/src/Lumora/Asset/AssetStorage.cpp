#include "LMPCH.h"

#include "AssetStorage.h"

namespace Lumora
{
	bool AssetStorage::HasAsset(AssetIdT assetId) const
	{
		LM_PROFILE_FUNCTION();
		LM_LOCK_READ(m_AssetRecordsMutex);

		auto it = m_AssetRecords.find(assetId);
		if (it != m_AssetRecords.end())
		{
			return true;
		}
		return false;
	}

	bool AssetStorage::IsLoaded(AssetIdT assetId) const
	{
		LM_PROFILE_FUNCTION();
		LM_LOCK_READ(m_AssetRecordsMutex);

		auto it = m_AssetRecords.find(assetId);
		LM_CORE_ASSERT(it != m_AssetRecords.end(), "Asset Record not Found")

		auto& assetRecord = it->second;
		return assetRecord->IsLoaded();
	}

	Ref<AssetRecord> AssetStorage::GetAssetRecord(AssetIdT assetId)
	{
		LM_PROFILE_FUNCTION();
		LM_LOCK_READ(m_AssetRecordsMutex);

		auto it = m_AssetRecords.find(assetId);
		LM_CORE_ASSERT(it != m_AssetRecords.end(), "Asset Record not Found")

		return it->second;
	}

	void AssetStorage::AddAssetRecord(Ref<AssetRecord> assetRecord)
	{
		LM_PROFILE_FUNCTION();
		LM_LOCK_WRITE(m_AssetRecordsMutex);

		m_AssetRecords[assetRecord->GetAssetId()] = std::move(assetRecord);
	}

	void AssetStorage::RemoveAssetRecord(AssetIdT assetId)
	{
		LM_PROFILE_FUNCTION();
		LM_LOCK_WRITE(m_AssetRecordsMutex);

		auto it = m_AssetRecords.find(assetId);
		LM_CORE_ASSERT(it != m_AssetRecords.end(), "Asset Record not Found")

		m_AssetRecords.erase(it);
	}

	void AssetStorage::Unload(AssetIdT assetId)
	{
		LM_PROFILE_FUNCTION();
		LM_LOCK_READ(m_AssetRecordsMutex);

		auto it = m_AssetRecords.find(assetId);
		LM_CORE_ASSERT(it != m_AssetRecords.end(), "Asset Record not Found")

		it->second->Unload();
	}

	bool AssetStorage::HasDefaultAsset(std::type_index type)
	{
		LM_PROFILE_FUNCTION();
		LM_LOCK_READ(m_DefaultAssetsMutex);
	
		auto it = m_DefaultAssets.find(type);
		if (it != m_DefaultAssets.end())
		{
			return true;
		}
		return false;
	}
	
	Ref<AssetRecord> AssetStorage::GetDefaultAssetRecord(std::type_index type)
	{
		LM_PROFILE_FUNCTION();
		LM_LOCK_READ(m_DefaultAssetsMutex);
	
		auto it = m_DefaultAssets.find(type);
		LM_CORE_ASSERT(it != m_DefaultAssets.end(), "Default Asset Record not Found")
	
		return it->second;
	}
	
	void AssetStorage::SetDefaultAssetRecord(std::type_index type, Ref<AssetRecord> assetRecord)
	{
		LM_PROFILE_FUNCTION();
		LM_LOCK_WRITE(m_DefaultAssetsMutex);
	
		m_DefaultAssets[type] = std::move(assetRecord);
	}
	
	void AssetStorage::RemoveDefaultAssetRecord(std::type_index type)
	{
		LM_PROFILE_FUNCTION();
		LM_LOCK_WRITE(m_DefaultAssetsMutex);
	
		auto it = m_DefaultAssets.find(type);
		LM_CORE_ASSERT(it != m_DefaultAssets.end(), "Default Asset Record not Found")
	
		m_DefaultAssets.erase(it);
	}
	
	void AssetStorage::UnloadAll()
	{
		LM_PROFILE_FUNCTION();
		LM_LOCK_READ(m_AssetRecordsMutex);
		for (auto& record : std::views::values(m_AssetRecords))
		{
			record->Unload();
		}
	}
	
	void AssetStorage::Clear()
	{
		LM_PROFILE_FUNCTION();
		{
			LM_LOCK_WRITE(m_AssetRecordsMutex);
			m_AssetRecords.clear();
		}
		{
			LM_LOCK_WRITE(m_DefaultAssetsMutex);
			m_DefaultAssets.clear();
		}
	}
}