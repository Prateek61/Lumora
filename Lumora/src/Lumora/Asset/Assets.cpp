#include "LMPCH.h"
#include "Assets.h"
#include "Lumora/Core/Application.h"

namespace Lumora
{
	AssetManager& Assets::GetAssetManager()
	{
		return Application::Get().GetAssetManager();
	}

	AssetIdT Assets::Register(const std::filesystem::path& assetPropsFile)
	{
		return GetAssetManager().RegisterAsset(assetPropsFile);
	}

	AssetIdT Assets::Register(const Ref<AssetProps>& props)
	{
		return GetAssetManager().RegisterAsset(props);
	}

	bool Assets::Valid(AssetIdT id)
	{
		return GetAssetManager().IsValid(id);
	}

	AssetIdT Assets::GetId(std::string name)
	{
		return GetAssetManager().GetAssetId(name);
	}

}