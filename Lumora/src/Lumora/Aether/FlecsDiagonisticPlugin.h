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

		void Build(Core::Application& app) override;

		const char* GetName() const override { return "FlecsDiagnosticPlugin"; }

	private:
		uint16_t m_Port;
	};
}