#include "LMPCH.h"
#include "FlecsDiagnosticPlugin.h"

namespace Lumora::Aether
{
	void FlecsDiagnosticPlugin::Build(Core::Application& app)
	{
		LM_PROFILE_FUNCTION();

		auto& world = app.GetWorld().Raw();
		world.import <flecs::stats>();
		world.set<flecs::Rest>({.port = m_Port});
		LM_CORE_INFO("FlecsDiagnosticPlugin: https://flecs.dev/explorer?remote=true&host=localhost:{}", m_Port);
	}
}
