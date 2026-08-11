#include "Lumora.h"
#include "Glimmer.h"
#include "Spark.h"
#include "Text.h"

using namespace Lumora;

struct Config
{
	Flux::WindowProps WindowSettings;
	Glyph::ImGuiSettings ImGuiSettings;
	std::string RenderAPI = "OpenGL";
	bool UseGlimmer = true;
	bool GlimmerImGui = true;
	bool DiagnosticPlugin = false;
};
LM_REFLECTABLE(Config, WindowSettings, ImGuiSettings, RenderAPI, UseGlimmer, GlimmerImGui, DiagnosticPlugin);

namespace
{
	Lumen::RenderAPI ParseRenderAPI(const std::string& api)
	{
		if (api == "OpenGL")
			return Lumen::RenderAPI::OpenGL;
		if (api == "Vulkan")
			return Lumen::RenderAPI::Vulkan;

		LM_LOG_WARN("Unknown Render API \"{}\", defaulting to OpenGL", api);
		return Lumen::RenderAPI::OpenGL;
	}
}

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

	// API Choice
	config.WindowSettings.API = ParseRenderAPI(config.RenderAPI);
	const bool imgui_available = config.WindowSettings.API == Lumen::RenderAPI::OpenGL;

	Core::Application app = Core::Application::Create();
	app.AddPlugin<Flux::WindowPlugin>(config.WindowSettings);
	app.AddPlugin<Lumen::RendererPlugin>();
	if (imgui_available)
		app.AddPlugin<Glyph::ImGuiPlugin>(config.ImGuiSettings);
	app.AddPlugin<Lumen::Renderer2DPlugin>();
	app.AddPlugin<Rune::LuaSerializerPlugin>();
	app.AddPlugin<Atlas::AssetPlugin>(Atlas::AssetSettings{.AssetRoot = "../Assets/", .HotReload = true});

	if (config.UseGlimmer)
	{
		app.AddPlugin<Glimmer>(config.GlimmerImGui && imgui_available);
	}
	else if (imgui_available)
	{
		app.AddPlugin<Spark>();
	}
	else
	{
		LM_LOG_WARN("Spark needs ImGui, which has no {} backend yet. Running Glimmer instead", config.RenderAPI);
		app.AddPlugin<Glimmer>(false);
	}

	if (config.DiagnosticPlugin)
		app.AddPlugin<Aether::FlecsDiagnosticPlugin>();

	app.Run();
}
