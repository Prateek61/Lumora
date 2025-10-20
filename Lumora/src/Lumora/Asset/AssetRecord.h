#pragma once

#include "Lumora/Common/Base.h"
#include "Lumora/Asset/AssetCommon.h"

namespace Lumora
{
	class AssetRecord
	{
	public:
		AssetRecord(AssetIdT assetId, std::string name, Ref<Asset> assetPtr);

		// Accessors
		Ref<Asset> Get(AssetVersionT& outVersion) const;
		Ref<Asset> Get() const;
		AssetVersionT GetVersion() const;
		AssetIdT GetAssetId() const { return m_AssetId; }
		const std::string& GetName() const { return m_Name; }
		bool IsLoaded(AssetVersionT& outVersion) const;
		bool IsLoaded() const;

	private:
		Ref<Asset> m_AssetPtr{ nullptr };
		const AssetIdT m_AssetId;
		const std::string m_Name;
		AssetVersionT m_AssetVersion{ 0 };

		mutable RWMutex m_Mutex;

	private:
		friend class AssetManager;
		friend class AssetStorage;
		void UpdateAsset(Ref<Asset> newAsset, AssetVersionT& outVersion);
		void UpdateAsset(Ref<Asset> newAsset);
		void Unload();
	};
}
