#pragma once

#include "Lumora/Core/Application.h"

namespace Lumora
{
	// To be defined in CLIENT
	extern Application* CreateApplication(ApplicationCommandLineArgs args);
}

int main(int argc, char** argv)
{
	Lumora::Log::Init();

	LM_PROFILE_BEGIN_SESSION("Startup", "Startup.profile.json");
	Lumora::ApplicationCommandLineArgs args = { argc, argv };
	auto app = Lumora::CreateApplication(args);
	LM_PROFILE_END_SESSION();

	LM_PROFILE_BEGIN_SESSION("Runtime", "Runtime.profile.json");
	app->Run();
	LM_PROFILE_END_SESSION();

	LM_PROFILE_BEGIN_SESSION("Shutdown", "Suntdown.profile.json");
	{
		LM_PROFILE_SCOPE("Application Shutdown");
		delete app;
	}
	LM_PROFILE_END_SESSION();

	return 0;
}