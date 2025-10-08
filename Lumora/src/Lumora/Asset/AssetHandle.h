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
		explicit AssetHandle(Ref<AssetRecord> assetRecord, AssetManager* assetManager)
			: m_AssetVersion(assetRecord->GetVersion()),
			  m_CachedAsset(StaticRefCast<T>(assetRecord->Get())),
			  m_AssetId(assetRecord->GetAssetId()),
			  m_AssetManager(assetManager),
			  m_AssetRecord(std::move(assetRecord))
		{
			LM_CORE_ASSERT(m_AssetRecord, "AssetRecord is null")
		}

		bool Update();
		Ref<T> Get();
		bool EnsureReady();

		// Accessors
		AssetIdT GetAssetId() const { return m_AssetId; }
		AssetManager& GetAssetManager() const { return *m_AssetManager; }

		// Operators
		operator bool() { return EnsureReady(); }
		Ref<T> operator->() { return Get(); }
		Ref<T> operator*() { return Get(); }

	private:
		AssetVersionT m_AssetVersion;
		Ref<T> m_CachedAsset;
		AssetIdT m_AssetId;
		AssetManager* m_AssetManager;
		Ref<AssetRecord> m_AssetRecord;

	private:
		bool Load();
	};
}

#include "Lumora/Asset/AssetManager.h"

// Template Implementations
namespace Lumora
{
	template <typename T>
		requires std::is_base_of_v<Asset, T>
	bool AssetHandle<T>::Load()
	{
		LM_PROFILE_FUNCTION();
		LM_CORE_ASSERT(m_AssetManager->IsValid(m_AssetId), "Invalid Asset Handle");

		return m_AssetManager->Load(*m_AssetRecord);
	}

	template <typename T>
		requires std::is_base_of_v<Asset, T>
	bool AssetHandle<T>::Update()
	{
		LM_PROFILE_FUNCTION();
		LM_CORE_ASSERT(m_AssetManager->IsValid(m_AssetId), "Invalid Asset Handle");

		auto latest_ver = m_AssetRecord->GetVersion();
		if ( m_AssetVersion != latest_ver )
		{
			Ref<T> asset = StaticRefCast<T>(m_AssetRecord->Get());

			if (!asset)
			{
				return false;
			}

			m_AssetVersion = latest_ver;
			m_CachedAsset = std::move(asset);
			return true;
		}
		return false;
	}

	template <typename T>
		requires std::is_base_of_v<Asset, T>
	Ref<T> AssetHandle<T>::Get()
	{
		LM_PROFILE_FUNCTION();
		LM_CORE_ASSERT(m_AssetManager->IsValid(m_AssetId), "Invalid Asset Handle");

		Update();
		if ( !m_CachedAsset )
		{
			Load();
			Update();
		}

		LM_CORE_ASSERT(m_CachedAsset, "Asset is not loaded")
		return m_CachedAsset;
	}

	template<typename T>
		requires std::is_base_of_v<Asset, T>
	bool AssetHandle<T>::EnsureReady()
	{
		LM_PROFILE_FUNCTION();

		if (m_CachedAsset)
		{
			return true;
		}

		Load();
		Update();
		return static_cast<bool>(m_CachedAsset);
	}
}
