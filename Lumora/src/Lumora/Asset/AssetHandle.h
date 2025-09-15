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

		bool Reload();
		bool Load();
		void Unload();
		bool Update();
		bool IsValid();
		bool IsLoaded();

		Ref<T> Get();

		// Accessors
		AssetIdT GetAssetId() const { return m_AssetId; }
		AssetManager& GetAssetManager() const { return *m_AssetManager; }

		// Operators
		operator bool() const { return IsValid(); }
		Ref<T> operator->() { return Get(); }
		Ref<T> operator*() { return Get(); }

	private:
		AssetVersionT m_AssetVersion;
		Ref<T> m_CachedAsset;
		AssetIdT m_AssetId;
		AssetManager* m_AssetManager;
		Ref<AssetRecord> m_AssetRecord;
	};
}

#include "Lumora/Asset/AssetManager.h"

// Template Implementations
namespace Lumora
{
	template <typename T>
		requires std::is_base_of_v<Asset, T>
	bool AssetHandle<T>::Reload()
	{
		LM_PROFILE_FUNCTION();
		LM_CORE_ASSERT(IsValid(), "Invalid Asset Handle");

		m_AssetManager->Reload(*m_AssetRecord);
		return IsLoaded();
	}

	template <typename T>
		requires std::is_base_of_v<Asset, T>
	bool AssetHandle<T>::Load()
	{
		LM_PROFILE_FUNCTION();
		LM_CORE_ASSERT(IsValid(), "Invalid Asset Handle");

		if ( IsLoaded() ) return true;
		m_AssetManager->Load(*m_AssetRecord);
		return IsLoaded();
	}

	template <typename T>
		requires std::is_base_of_v<Asset, T>
	void AssetHandle<T>::Unload()
	{
		LM_PROFILE_FUNCTION();
		LM_CORE_ASSERT(IsValid(), "Invalid Asset Handle");

		if ( !IsLoaded() ) return;
		m_AssetManager->Unload(*m_AssetRecord);
		m_CachedAsset = nullptr;
		m_AssetVersion = g_INVALID_ASSET_VERSION;
	}

	template <typename T>
		requires std::is_base_of_v<Asset, T>
	bool AssetHandle<T>::Update()
	{
		LM_PROFILE_FUNCTION();
		LM_CORE_ASSERT(IsValid(), "Invalid Asset Handle");

		if ( m_AssetVersion != m_AssetRecord->GetVersion() )
		{
			m_AssetVersion = m_AssetRecord->GetVersion();
			m_CachedAsset = StaticRefCast<T>(m_AssetRecord->Get());
			return true;
		}
		return false;
	}

	template <typename T>
		requires std::is_base_of_v<Asset, T>
	bool AssetHandle<T>::IsValid()
	{
		LM_PROFILE_FUNCTION();

		if ( m_AssetRecord && m_AssetManager && m_AssetManager->IsValid(m_AssetId) ) return true;
		return false;
	}

	template <typename T>
		requires std::is_base_of_v<Asset, T>
	bool AssetHandle<T>::IsLoaded()
	{
		LM_PROFILE_FUNCTION();
		LM_CORE_ASSERT(IsValid(), "Invalid Asset Handle")

		Update();
		return static_cast<bool>(m_CachedAsset);
	}

	template <typename T>
		requires std::is_base_of_v<Asset, T>
	Ref<T> AssetHandle<T>::Get()
	{
		LM_PROFILE_FUNCTION();
		LM_CORE_ASSERT(IsValid(), "Invalid Asset Handle");

		Update();
		if ( !m_CachedAsset )
		{
			Load();
		}

		LM_CORE_ASSERT(m_CachedAsset, "Asset is not loaded");
		return m_CachedAsset;
	}
}
