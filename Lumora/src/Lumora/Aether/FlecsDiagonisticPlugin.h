#pragma once

#include "Lumora/Aether/Base.h"
#include "Lumora/Core/Plugin.h"
#include "Lumora/Core/Application.h"

namespace Lumora::Aether
{
	class FlecsDiagnosticPlugin : public Core::Plugin
	{
	public:
		FlecsDiagnosticPlugin(uint16_t port = 27750) : m_Port(port) {}

		void Build(Core::Application& app) override 
		{ 
			LM_PROFILE_FUNCTION();

			auto& world = app.GetWorld().Raw();
			world.import <flecs::stats>();
			world.set<flecs::Rest>({.port=m_Port});
			LM_CORE_INFO("FlecsDiagnosticPlugin: https://flecs.dev/explorer?remore=true&host=localhost:{}", m_Port);
		}

		const char* GetName() const override { return "FlecsDiagnosticPlugin"; }

	private:
		uint16_t m_Port;
	};
}