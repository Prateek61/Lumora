#pragma once

#include "Lumora/Asset/AssetCommon.h"
#include "Lumora/Asset/Asset.h"
#include "Lumora/Asset/AssetRecord.h"

namespace Lumora
{
	template<typename T>
		requires std::is_base_of_v<Asset, T>
	class AssetHandle
	{
	public:
		AssetHandle();
		AssetHandle(Ref<AssetRecord> assetRecord, AssetManager* assetManager);

		Ref<T> Get();
		AssetIdT GetId() const;
		bool Updated(bool updateVersion = true);

		bool Load();
		void Reload();

		bool Cache();
		void Uncache();
		bool IsCached() const { return m_Cached; }

		bool IsHandleValid() const;
		operator bool() const { return IsHandleValid(); }
	private:
		AssetVersionT m_AssetVersion;
		Ref<T> m_CachedAsset;
		Ref<AssetRecord> m_AssetRecord;
		AssetManager* m_AssetManager;
		bool m_Cached = false;

	private:
		void UpdateVersion();
	};
}

// Template Implementations

#include "Lumora/Asset/AssetManager.h"
namespace Lumora
{
	template<typename T>
		requires std::is_base_of_v<Asset, T>
	AssetHandle<T>::AssetHandle()
		: m_AssetVersion(0),
		  m_CachedAsset(nullptr),
		  m_AssetRecord(nullptr),
		  m_AssetManager(nullptr)
	{
	}

	template<typename T>
		requires std::is_base_of_v<Asset, T>
	AssetHandle<T>::AssetHandle(Ref<AssetRecord> assetRecord, AssetManager* assetManager)
		: m_AssetVersion(assetRecord->GetVersion()),
		  m_CachedAsset(StaticRefCast<T>(assetRecord->Get())),
		  m_AssetRecord(std::move(assetRecord)),
		  m_AssetManager(assetManager)
	{
	}

	template<typename T>
		requires std::is_base_of_v<Asset, T>
	Ref<T> AssetHandle<T>::Get()
	{
		LM_PROFILE_FUNCTION();
		if (!IsHandleValid())
		{
			return nullptr;
		}

		if (m_Cached)	return m_CachedAsset;

		auto ref = m_AssetRecord->Get(m_AssetVersion);
		if (!ref && Load())
		{
			ref = m_AssetRecord->Get(m_AssetVersion);
		}

		return StaticRefCast<T>(ref);
	}

	template<typename T>
		requires std::is_base_of_v<Asset, T>
	AssetIdT AssetHandle<T>::GetId() const
	{
		LM_CORE_ASSERT(IsHandleValid(), "Invalid Asset Handle")

		return m_AssetRecord->GetAssetId();
	}

	template<typename T>
		requires std::is_base_of_v<Asset, T>
	bool AssetHandle<T>::Updated(bool updateVersion)
	{
		LM_PROFILE_FUNCTION();
		LM_CORE_ASSERT(IsHandleValid(), "Invalid Asset Handle")

		auto latest_ver = m_AssetRecord->GetVersion();
		if (m_AssetVersion == latest_ver)
		{
			return false;
		}

		if (updateVersion)
		{
			m_AssetVersion = latest_ver;
		}
		return true;
	}

	template<typename T>
		requires std::is_base_of_v<Asset, T>
	bool AssetHandle<T>::Load()
	{
		LM_PROFILE_FUNCTION();
		LM_CORE_ASSERT(IsHandleValid(), "Invalid Asset Handle")

		if (m_Cached && m_CachedAsset)	return true;
		if (m_AssetRecord->IsLoaded())
		{
			if (m_Cached)
			{
				m_CachedAsset = StaticRefCast<T>(m_AssetRecord->Get(m_AssetVersion));
			}
			return true;
		}

		m_AssetManager->Load(*m_AssetRecord);
		auto ref = m_AssetRecord->Get(m_AssetVersion);
		if (m_Cached) m_CachedAsset = StaticRefCast<T>(ref);
		return ref != nullptr;
	}

	template<typename T>
		requires std::is_base_of_v<Asset, T>
	void AssetHandle<T>::Reload()
	{
		LM_PROFILE_FUNCTION();
		LM_CORE_ASSERT(IsHandleValid(), "Invalid Asset Handle")

		m_AssetManager->Load(*m_AssetRecord);
		if (m_Cached)
		{
			if (m_AssetRecord->IsLoaded())
			{
				m_CachedAsset = StaticRefCast<T>(m_AssetRecord->Get(m_CachedAsset));
			}
		}
	}

	template <typename T>
		requires std::is_base_of_v<Asset, T>
	bool AssetHandle<T>::Cache()
	{
		LM_PROFILE_FUNCTION();

		if (m_Cached) return true;
		m_Cached = true;
		Load();
		return true;
	}

	template<typename T>
		requires std::is_base_of_v<Asset, T>
	void AssetHandle<T>::Uncache()
	{
		LM_PROFILE_FUNCTION();
		m_Cached = false;
		m_CachedAsset = nullptr;
	}

	template <typename T>
		requires std::is_base_of_v<Asset, T>
	bool AssetHandle<T>::IsHandleValid() const
	{
		return m_AssetManager && m_AssetRecord && m_AssetRecord->GetAssetId() != g_INVALID_ASSET_ID;
	}

	template <typename T>
		requires std::is_base_of_v<Asset, T>
	void AssetHandle<T>::UpdateVersion()
	{
		m_AssetVersion = m_AssetRecord->GetVersion();
	}
}
