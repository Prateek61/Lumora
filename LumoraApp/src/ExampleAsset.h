#pragma once

#include "Lumora.h"

struct ExampleAssetProps : Lumora::AssetProps
{
	std::string Data;
};
LM_VISITABLE_ASSET_PROPS(ExampleAssetProps, Data);

class ExampleAsset : public Lumora::Asset
{
public:
	ExampleAsset(std::string data)
		: Data(std::move(data))
	{
	}

	std::string Data;
};
inline Lumora::Ref<ExampleAsset> LoadExampleAsset(ExampleAssetProps& props)
{
	return Lumora::CreateRef<ExampleAsset>(props.Data);
}
LM_REGISTER_ASSET_TYPE("ExampleAsset", ExampleAsset, ExampleAssetProps, LoadExampleAsset);
