#pragma once

#include "Lumora/Asset/AssetCommon.h"
#include "Lumora/Asset/AssetRecord.h"

#include <typeindex>

namespace Lumora
{
	class AssetStorage
	{
	public:
		AssetStorage() = default;
		~AssetStorage() = default;

		// Assets
		bool HasAsset(AssetIdT assetId) const;
		Ref<AssetRecord> GetAssetRecord(AssetIdT assetId);
		void AddAssetRecord(Ref<AssetRecord> assetRecord);
		void RemoveAssetRecord(AssetIdT assetId);

		// Default Assets
		bool HasDefaultAsset(std::type_index type);
		Ref<AssetRecord> GetDefaultAssetRecord(std::type_index type);
		void SetDefaultAssetRecord(std::type_index type, Ref<AssetRecord> assetRecord);
		void RemoveDefaultAssetRecord(std::type_index type);

	private:
		AssetMap<AssetIdT, Ref<AssetRecord>> m_AssetRecords;
		AssetMap<std::type_index, Ref<AssetRecord>> m_DefaultAssets;

		mutable Lumora::RWMutex m_AssetRecordsMutex;
		mutable Lumora::RWMutex m_DefaultAssetsMutex; 

	private:
		void UnloadAll(); // Unload all assets but keep the records
		void Clear(); // Clear all records and assets

		friend class AssetManager;
	};
}