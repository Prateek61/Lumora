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
	Lumora::AssetReloader reloader;

	App(const Lumora::ApplicationProps& props)
		: Lumora::Application(props), reloader(props.AssetsDirectory, assetReloadCallback, metadataCallback)
	{
		reloader.StartWatching();

		reloader.WatchFile("test.local.lua", 1);
	}

	~App() override
	{
		reloader.StopWatching();
	}
};

Lumora::Application* Lumora::CreateApplication(ApplicationCommandLineArgs args)
{
	Lumora::LuaSerializer serializer;
	auto props = ApplicationProps::Get("../Assets/Config.lua", serializer);
	props.CommandLineArgs = args;
	return new App(props);
}
