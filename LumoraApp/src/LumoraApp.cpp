#include "Lumora.h"

#include "Lumora/Entrypoint.h"
#include "ExampleAsset.h"
#include "LumoraLayer.h"
#include "bgfx/bgfx.h"
#include "bx/math.h"

class App : public Lm::Application
{
public:
	App(const Lm::ApplicationProps& props)
		: Lm::Application(props)
	{
		PushLayer(new LumoraLayer());

		GetRendererContext().SetClearColor(0x9a9a9aff);
	}

	~App() override = default;

	void OnUpdate(Lm::TimeStep ts) override
	{
	}
};

Lm::Application* Lm::CreateApplication(ApplicationCommandLineArgs args)
{
	Lm::LuaSerializer serializer;
	auto props = ApplicationProps::Get("..\\Assets\\Config.lua", serializer);
	props.CommandLineArgs = args;

	// Make sure to Initialize the Logger
	Lm::Log::Init(props.LoggerConfig);

	return new App(props);
}
