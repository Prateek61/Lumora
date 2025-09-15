#include "Lumora.h"

#include "Lumora/Entrypoint.h"
#include "ExampleAsset.h"

class App : public Lumora::Application
{
public:
	App(const std::filesystem::path& configFile, Lumora::ApplicationCommandLineArgs args)
		: Lumora::Application(configFile, args)
	{
		auto id = Lumora::Assets::Register("Example.local.asset.lua");

		auto handle = Lumora::Assets::Get<ExampleAsset>("JustExample");
		LM_CORE_TRACE("Data: {}", handle->Data);
	}
};

Lumora::Application* Lumora::CreateApplication(ApplicationCommandLineArgs args)
{
	return new App("../Assets/Config.lua", args);
}
