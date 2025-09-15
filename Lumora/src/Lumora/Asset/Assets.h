#pragma once

#include "Lumora/Core/Application.h"

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
		static void Register(std::filesystem::path assetPropsFile);
		static void Register(const Ref<AssetProps> props);
	};
}