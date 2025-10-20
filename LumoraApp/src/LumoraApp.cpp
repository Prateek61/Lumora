#include "Lumora.h"

#include "Lumora/Entrypoint.h"
#include "ExampleAsset.h"

class App : public Lumora::Application
{
public:
	App(const Lumora::ApplicationProps& props)
		: Lumora::Application(props)
	{
		Handle = Lumora::Assets::Get<ExampleAsset>("JustExample");
		LM_INFO("ExampleAsset Data: {}", Handle.Get()->Data);

		GetRendererContext().SetClearColor(0x9a9a9aff);
	}

	~App() override = default;

	void OnUpdate(Lumora::TimeStep ts) override
	{
		if (Handle && Handle.Updated())
		{
			LM_INFO("ExampleAsset Updated!, New Data: {}", Handle.Get()->Data);
		}
	}

	Lumora::AssetHandle<ExampleAsset> Handle;
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
