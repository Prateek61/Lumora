#include "Lumora.h"
#include "Glimmer.h"
#include "Spark.h"

using namespace Lumora;

int main()
{
	Core::Application app = Core::Application::Create();
	app.AddPlugin<Flux::WindowPlugin>(Flux::WindowProps{.Title = "Silly Linguine Cat Simulator Deluxe Online", .Width = 1280, .Height = 720});
	app.AddPlugin<Lumen::RendererPlugin>();
	app.AddPlugin<Glyph::ImGuiPlugin>(Glyph::ImGuiSettings{.IniFilename = "ImGui.local.ini"});
	app.AddPlugin<Lumen::Renderer2DPlugin>();
	app.AddPlugin<Glimmer>();
	//app.AddPlugin<Spark>();

	app.AddPlugin<Aether::FlecsDiagnosticPlugin>();

	app.Run();
}
