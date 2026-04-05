#include "Lumora.h"
#include "Glimmer.h"

using namespace Lm;

int main()
{
	Core::Application app = Core::Application::Create();
	app.AddPlugin<Flux::FluxPlugin>().AddPlugin<Glimmer>();
	
	app.Run();
}
