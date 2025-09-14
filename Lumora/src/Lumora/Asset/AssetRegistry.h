#pragma once

#include "Lumora/Asset/AssetCommon.h"
#include "Lumora/Asset/AssetProps.h"

namespace Lumora
{
	/// Manages asset properties
	class AssetRegistry
	{
	public:
		AssetIdT GetAssetId(const std::string& name) const;
		Ref<AssetProps> GetAssetProps(AssetIdT id) const;

		AssetIdT RegisterAsset(const Ref<AssetProps>& props);
		void RegisterAsset(AssetIdT id, const Ref<AssetProps>& props);
		void UnregisterAsset(AssetIdT id);

	private:
		AssetMap<std::string, AssetIdT> m_NameToIdMap;
		AssetMap<AssetIdT, Ref<AssetProps>> m_AssetProps;

		mutable LM_MUTEX(m_NameToIdMutex);
		mutable LM_MUTEX(m_IdToPropsMutex);
	private:
		void RegisterAssetInternal(AssetIdT id, const Ref<AssetProps>& props);
	};
}