#pragma once

#include "Lumora/Asset/AssetManager.h"

namespace Lumora
{
	class Assets
	{
	public:
		static AssetManager& GetAssetManager();
		template<typename T>
		static AssetHandle<T> Get(AssetIdT assetId);
		template<typename T>
		static AssetHandle<T> Get(const std::string& name);
		static AssetIdT Register(const std::filesystem::path& assetPropsFile);
		static AssetIdT Register(const Ref<AssetProps>& props);
		static bool Valid(AssetIdT id);
		static AssetIdT GetId(std::string name);
		static std::filesystem::path GetAssetRoot() { return GetAssetManager().GetAssetRoot(); }
		static std::filesystem::path GetFullAssetPath(const std::filesystem::path& relativePath) { return GetAssetManager().GetFullAssetPath(relativePath); }
	};
}

// Include the implementation of the template functions
namespace Lumora
{
	template<typename T>
	AssetHandle<T> Assets::Get(AssetIdT assetId)
	{
		return GetAssetManager().GetAssetHandle<T>(assetId);
	}

	template<typename T>
	AssetHandle<T> Assets::Get(const std::string& name)
	{
		return GetAssetManager().GetAssetHandle<T>(name);
	}
}