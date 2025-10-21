#include "Lumora.h"

#include "Lumora/Entrypoint.h"
#include "ExampleAsset.h"
#include "LumoraLayer.h"
#include "bgfx/bgfx.h"
#include "bx/math.h"

class App : public Lumora::Application
{
public:
	App(const Lumora::ApplicationProps& props)
		: Lumora::Application(props)
	{
		PushLayer(new LumoraLayer());

		GetRendererContext().SetClearColor(0x9a9a9aff);
	}

	~App() override = default;

	void OnUpdate(Lumora::TimeStep ts) override
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
