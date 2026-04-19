#include "LMPCH.h"
#include "FlecsDiagonisticPlugin.h"

namespace Lumora::Aether
{
	void FlecsDiagnosticPlugin::Build(Core::Application& app)
	{
		LM_PROFILE_FUNCTION();

		auto& world = app.GetWorld().Raw();
		world.import <flecs::stats>();
		world.set<flecs::Rest>({.port = m_Port});
		LM_CORE_INFO("FlecsDiagnosticPlugin: https://flecs.dev/explorer?remore=true&host=localhost:{}", m_Port);
	}
}
