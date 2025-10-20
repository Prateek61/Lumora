#include "Lumora.h"

#include "Lumora/Entrypoint.h"
#include "ExampleAsset.h"
#include "bgfx/bgfx.h"
#include "bx/math.h"

class App : public Lumora::Application
{
public:
	App(const Lumora::ApplicationProps& props)
		: Lumora::Application(props)
	{
		Handle = Lumora::Assets::Get<ExampleAsset>("JustExample");
		bgfx::setDebug(BGFX_DEBUG_TEXT);

		GetRendererContext().SetClearColor(0x9a9a9aff);

		auto str = GetSerializer().SerializeToLuaScript<Lumora::ApplicationProps>(props);
		LM_LOG_INFO("Serialized ApplicationProps:\n{}", str);
	}

	~App() override = default;

	void OnUpdate(Lumora::TimeStep ts) override
	{
		/*cum_time += ts.GetSeconds();
		float sample = cum_time / 2.5f;
		uint8_t red = static_cast<uint8_t>(bx::clamp(bx::sin(sample) * 0.5f + 0.5f, 0.0f, 1.0f) * 255);
		uint8_t green = static_cast<uint8_t>(bx::clamp(bx::sin(sample * 2) * 0.5f + 0.5f, 0.0f, 1.0f) * 255);
		uint8_t blue = static_cast<uint8_t>(bx::clamp(bx::sin(sample * 0.5f) * 0.5f + 0.5f, 0.0f, 1.0f) * 255);
		uint32_t color = (red << 24) | (green << 16) | (blue << 8) | 0xff;
		GetRendererContext().SetClearColor(color);*/

		// Draw text on the screen using bgfx
		bgfx::dbgTextClear();
		bgfx::dbgTextPrintf(0, 1, 0x0f, "Lumora Application - Example");
		bgfx::dbgTextPrintf(0, 2, 0x0f, "ExampleAsset Data: %s", Handle.Get()->Data.c_str());

		bgfx::dbgTextPrintf(0, 4, 0x0f, "FPS: %f", 1.0f / ts.GetSeconds());
	}

	Lumora::AssetHandle<ExampleAsset> Handle;
	float cum_time = 0.0f;
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
