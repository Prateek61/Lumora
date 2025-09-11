#include "Lumora.h"

#include "Lumora/Entrypoint.h"

Lumora::Application* Lumora::CreateApplication(ApplicationCommandLineArgs args)
{
	return new Application("../Assets/Config.lua", args);
}
