#pragma once

#include "Lumora/Common/Base.h"
#include "Lumora/Asset/AssetCommon.h"

namespace Lumora
{
	// Stable container owned by AssetManager
	class AssetRecord
	{
	public:
		AssetRecord(AssetIdT assetId, std::string name, Ref<Asset> assetPtr)
			: m_AssetPtr(std::move(assetPtr)),
			  m_AssetId(assetId),
			  m_Name(std::move(name))
		{
		}

		// Accessors
		Ref<Asset> Get() const { return m_AssetPtr.load(std::memory_order_relaxed); }
		AssetVersionT GetVersion() const { return m_AssetVersion.load(std::memory_order_relaxed); }
		AssetIdT GetAssetId() const { return m_AssetId; }
		bool IsValid() const { return m_AssetPtr.load(std::memory_order_relaxed) != nullptr; }

	private:
		Atomic<Ref<Asset>> m_AssetPtr{ nullptr };
		const AssetIdT m_AssetId;
		std::string m_Name;
		Atomic<AssetVersionT> m_AssetVersion{0};


		friend class AssetManager;
		friend class AssetStorage;

		void UpdateAsset(Ref<Asset> newAsset)
		{
			m_AssetPtr.store(std::move(newAsset), std::memory_order_relaxed);
			m_AssetVersion.fetch_add(1, std::memory_order_relaxed);
		}

		void Unload()
		{
			UpdateAsset(nullptr);
		}
	};
}
