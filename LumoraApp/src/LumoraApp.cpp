#include "Lumora.h"

#include "Lumora/Entrypoint.h"
#include "ExampleAsset.h"

namespace
{
	void assetReloadCallback(Lumora::AssetIdT id)
	{
		LM_TRACE("Reloading Asset: {0}", static_cast<uint64_t>(id));
	}

	void metadataCallback(const std::filesystem::path& path)
	{
		LM_TRACE("Meta file changed: {0}", path.string());
	}
}

class App : public Lumora::Application
{
public:
	App(const Lumora::ApplicationProps& props)
		: Lumora::Application(props)
	{
		auto id = Lumora::Assets::Register("Example.local.asset.lua");

		GetRendererContext().SetClearColor(0x9a9a9aff);
	}

	~App() override
	{
	}
};

Lumora::Application* Lumora::CreateApplication(ApplicationCommandLineArgs args)
{
	Lumora::LuaSerializer serializer;
	auto props = ApplicationProps::Get("..\\Assets\\Config.lua", serializer);
	props.CommandLineArgs = args;

	// Make sure to Initialize the Logger
	Log::Init(props.LoggerConfig);

	return new App(props);
}
