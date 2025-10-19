#include "LMPCH.h"
#include "AssetRegistry.h"
#include "AssetTypeRegistry.h"
#include "AssetManager.h"

namespace Lumora
{
	void AssetRegistry::RegisterAssetInternal(AssetIdT id, const Ref<AssetProps>& props)
	{
		LM_PROFILE_FUNCTION();
		LM_CORE_ASSERT(id != g_INVALID_ASSET_ID, "Invalid Asset ID")
		props->AssetId = id;
		{
			LM_LOCK_WRITE(m_NameToIdMutex);

			m_NameToIdMap[props->Name] = id;
		}

		{
			LM_LOCK_WRITE(m_IdToPropsMutex);

			m_AssetProps[id] = props;
		}

		LM_CORE_ASSETS_INFO("Registered Asset: {} with ID {}", props->Name, static_cast<std::string>(id));
	}

	AssetIdT AssetRegistry::GetAssetId(const std::string& name) const
	{
		LM_PROFILE_FUNCTION();
		LM_LOCK_READ(m_NameToIdMutex);

		auto it = m_NameToIdMap.find(name);
		if (it != m_NameToIdMap.end())
		{
			return it->second;
		}
		return g_INVALID_ASSET_ID;
	}

	Ref<AssetProps> AssetRegistry::GetAssetProps(AssetIdT id) const
	{
		LM_PROFILE_FUNCTION();
		LM_LOCK_READ(m_IdToPropsMutex);

		auto it = m_AssetProps.find(id);
		if (it != m_AssetProps.end())
		{
			return it->second;
		}
		return nullptr;
	}

	AssetIdT AssetRegistry::RegisterAsset(const Ref<AssetProps>& props)
	{
		LM_PROFILE_FUNCTION();

		auto id = GenerateAssetId();
		RegisterAssetInternal(id, props);
		return id;
	}

	void AssetRegistry::RegisterAsset(const Ref<AssetProps>& props, AssetIdT id)
	{
		LM_PROFILE_FUNCTION();

		RegisterAssetInternal(id, props);
	}

	void AssetRegistry::UnregisterAsset(AssetIdT id)
	{
		LM_PROFILE_FUNCTION();

		std::string name;

		{
			LM_LOCK_WRITE(m_IdToPropsMutex);

			auto it = m_AssetProps.find(id);
			if (it != m_AssetProps.end())
			{
				name = it->second->Name;
				m_AssetProps.erase(it);
			}
		}

		if (!name.empty())
		{
			LM_LOCK_WRITE(m_NameToIdMutex);
			auto it = m_NameToIdMap.find(name);
			if (it != m_NameToIdMap.end())
			{
				m_NameToIdMap.erase(it);
			}
		}
	}
}