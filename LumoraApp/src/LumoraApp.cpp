#include "Lumora.h"

#include "Lumora/Entrypoint.h"

class App : public Lumora::Application
{
public:
	App(const std::filesystem::path& configFile, Lumora::ApplicationCommandLineArgs args)
		: Lumora::Application(configFile, args)
	{
		
	}
};

Lumora::Application* Lumora::CreateApplication(ApplicationCommandLineArgs args)
{
	return new App("../Assets/Config.lua", args);
}
