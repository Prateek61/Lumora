#include "Lumora.h"
#include "Glimmer.h"

using namespace Lm;

int main()
{
	Core::Application app = Core::Application::Create();
	app.AddPlugin<Flux::WindowPlugin>(Flux::WindowProps{.Title = "Silly Linguine Cat Simulator Deluxe Online"});
	app.AddPlugin<Lumen::RendererPlugin>();
	app.AddPlugin<Glyph::ImGuiPlugin>(Glyph::ImGuiSettings{.ShowDemoWindow = true, .IniFilename = "ImGui.local.ini"});
	app.AddPlugin<Lumen::Renderer2DPlugin>();
	app.AddPlugin<Glimmer>();

	app.AddPlugin<Aether::FlecsDiagnosticPlugin>();

	app.Run();
}
