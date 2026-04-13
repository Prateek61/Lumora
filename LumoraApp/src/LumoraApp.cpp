#include "Lumora.h"
#include "Glimmer.h"

using namespace Lm;

int main()
{
	Core::Application app = Core::Application::Create();
	app.AddPlugin<Flux::WindowPlugin>(Flux::WindowProps{.Title = "Silly Linguine Cat Simulator Deluxe Online"})
		.AddPlugin<Lumen::RendererPlugin>()
		.AddPlugin<Lumen::Renderer2DPlugin>()
		.AddPlugin<Glimmer>();

#ifdef LM_DEBUG
	app.AddPlugin<Aether::FlecsDiagnosticPlugin>(8000);
#endif

	app.Run();
}
