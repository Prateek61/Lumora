#pragma once

#include "Lumora/Asset/AssetCommon.h"
#include "Lumora/Asset/Asset.h"
#include "Lumora/Asset/AssetRecord.h"

namespace Lumora
{
	template <typename T>
		requires std::is_base_of_v<Asset, T>
	class AssetHandle
	{
	public:
		explicit AssetHandle(Ref<AssetRecord> assetRecord)
			: m_AssetRecord(std::move(assetRecord)),
			  m_AssetVersion(assetRecord->GetVersion()),
			  m_CachedAsset(assetRecord->Get()),
			  m_AssetId(assetRecord->GetAssetId())
		{
			LM_CORE_ASSERT(m_AssetRecord, "AssetRecord is null");
		}

		bool Reload()
		{
			auto current_version = m_AssetRecord->GetVersion();
			if (current_version != m_AssetVersion)
			{
				m_AssetVersion = current_version;
				m_CachedAsset = StaticRefCast<T>(m_AssetRecord->Get());
				return true;
			}
			return false;
		}

		bool IsUpToDate() const
		{
			auto current_version = m_AssetRecord->GetVersion();
			return current_version == m_AssetVersion;
		}

		Ref<T> Get()
		{
			Reload();
			return m_CachedAsset;
		}

		Ref<T> GetCached()
		{
			return m_CachedAsset;
		}

	private:
		Ref<AssetRecord> m_AssetRecord;
		AssetVersionT m_AssetVersion;
		Ref<T> m_CachedAsset;
		AssetIdT m_AssetId;
	};
}
