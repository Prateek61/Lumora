#include "Lumora.h"

#include "Lumora/Entrypoint.h"
#include "ExampleAsset.h"

class App : public Lumora::Application
{
public:
	App(const Lumora::ApplicationProps& props)
		: Lumora::Application(props)
	{
		auto id = Lumora::Assets::Register("Testing.local.meta.lua");
		if (Lumora::Assets::Valid(id))
		{
			Handle = Lumora::Assets::Get<ExampleAsset>(id);
			if (!Handle.Load())
			{
				LM_ERROR("Failed to load ExampleAsset");
			}
		}

		GetRendererContext().SetClearColor(0x9a9a9aff);
	}

	~App() override
	{
	}

	void OnUpdate(Lumora::TimeStep ts) override
	{
		if (Handle.Updated())
		{
			auto ref = Handle.Get();
			if (ref)
			{
				LM_INFO("ExampleAsset Updated! New Data: {}", ref->Data);
			}
			else
			{
				LM_ERROR("Failed to get ExampleAsset after update!");
			}
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
