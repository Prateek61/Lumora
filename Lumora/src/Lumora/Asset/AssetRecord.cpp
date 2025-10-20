#include "LMPCH.h"
#include "AssetRecord.h"

namespace Lumora
{
	AssetRecord::AssetRecord(AssetIdT assetId, std::string name, Ref<Asset> assetPtr)
		: m_AssetPtr(std::move(assetPtr)),
		  m_AssetId(assetId),
		m_Name(std::move(name))
	{
	}

	Ref<Asset> AssetRecord::Get(AssetVersionT& outVersion) const
	{
		LM_PROFILE_FUNCTION();

		LM_LOCK_READ(m_Mutex);

		outVersion = m_AssetVersion;
		return m_AssetPtr;
	}

	Ref<Asset> AssetRecord::Get() const
	{
		LM_PROFILE_FUNCTION();
		LM_LOCK_READ(m_Mutex);
		return m_AssetPtr;
	}

	AssetVersionT AssetRecord::GetVersion() const
	{
		LM_PROFILE_FUNCTION();
		LM_LOCK_READ(m_Mutex);
		return m_AssetVersion;
	}

	bool AssetRecord::IsLoaded(AssetVersionT& outVersion) const
	{
		LM_PROFILE_FUNCTION();
		LM_LOCK_READ(m_Mutex);
		outVersion = m_AssetVersion;
		return m_AssetPtr != nullptr;
	}

	bool AssetRecord::IsLoaded() const
	{
		LM_PROFILE_FUNCTION();
		LM_LOCK_READ(m_Mutex);
		return m_AssetPtr != nullptr;
	}

	void AssetRecord::UpdateAsset(Ref<Asset> newAsset, AssetVersionT& outVersion)
	{
		LM_PROFILE_FUNCTION();
		LM_LOCK_WRITE(m_Mutex);

		m_AssetPtr = std::move(newAsset);
		m_AssetVersion++;
		outVersion = m_AssetVersion;
	}

	void AssetRecord::UpdateAsset(Ref<Asset> newAsset)
	{
		LM_PROFILE_FUNCTION();
		LM_LOCK_WRITE(m_Mutex);
		m_AssetPtr = std::move(newAsset);
		m_AssetVersion++;
	}

	void AssetRecord::Unload()
	{
		LM_PROFILE_FUNCTION();
		LM_LOCK_WRITE(m_Mutex);
		m_AssetPtr = nullptr;
		m_AssetVersion++;
	}
}