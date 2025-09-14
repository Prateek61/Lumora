#include "Lumora.h"

#include "Lumora/Entrypoint.h"

class TestAssetProps : public Lumora::AssetProps
{
};
LM_VISITABLE_ASSET_PROPS(TestAssetProps);
class TestAsset : public Lumora::Asset
{
};
Lumora::Ref<TestAsset> LoadTestAsset(TestAssetProps& props)
{
	return Lumora::CreateRef<TestAsset>();
}
LM_REGISTER_ASSET_TYPE("TestAsset", TestAsset, TestAssetProps, LoadTestAsset)

class App : public Lumora::Application
{
public:
	App(const std::filesystem::path& configFile, Lumora::ApplicationCommandLineArgs args)
		: Lumora::Application(configFile, args)
	{
		auto props = Lumora::AssetProps::DeSerialize("../Assets/TestAsset.local.lua", GetSerializer());
		LM_TRACE(props->ToString());
	}
};

Lumora::Application* Lumora::CreateApplication(ApplicationCommandLineArgs args)
{
	return new App("../Assets/Config.lua", args);
}
