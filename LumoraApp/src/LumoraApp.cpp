#include "Lumora.h"
#include "Glimmer.h"
#include "Spark.h"

using namespace Lumora;

struct Config
{
	Flux::WindowProps WindowSettings;
	Glyph::ImGuiSettings ImGuiSettings;
	bool UseGlimmer = true;
	bool GlimmerImGui = true;
};
VISITABLE_STRUCT(Config, WindowSettings, ImGuiSettings, UseGlimmer, GlimmerImGui);

int main()
{
	Log::Init();

	Rune::LuaSerializer serializer;
	auto config_opt = serializer.DeserializeFromFile<Config>("../Assets/Config.lua");
	if (!config_opt)
	{
		LM_CORE_ERROR("Failed to load config, using defaults");
		config_opt = Config{};
	}
	auto& config = config_opt.value();

	Core::Application app = Core::Application::Create();
	app.AddPlugin<Flux::WindowPlugin>(config.WindowSettings);
	app.AddPlugin<Lumen::RendererPlugin>();
	app.AddPlugin<Glyph::ImGuiPlugin>(config.ImGuiSettings);
	app.AddPlugin<Lumen::Renderer2DPlugin>();
	app.AddPlugin<Rune::LuaSerializerPlugin>();

	if (config.UseGlimmer)
		app.AddPlugin<Glimmer>(config.GlimmerImGui);
	else
		app.AddPlugin<Spark>();

	app.AddPlugin<Aether::FlecsDiagnosticPlugin>();

	app.Run();
}
