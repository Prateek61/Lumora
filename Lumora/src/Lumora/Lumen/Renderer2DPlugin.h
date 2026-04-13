#pragma once

#include "Lumora/Core/Plugin.h"
#include "Lumora/Aether/System.h"

namespace Lumora::Lumen
{
	class Renderer2DPlugin : public Core::Plugin
	{
	public:
		Renderer2DPlugin() = default;

		void Build(Core::Application& app) override;
		void Cleanup(Core::Application& app) override;

		void AddDependencies(Core::DependencyList& dependencies) override;

		const char* GetName() const override { return "Renderer2DPlugin"; }

	private:
		Aether::System m_BeginFrameSystem;
		Aether::System m_EndFrameSystem;
		Aether::System m_UpdateStatsSystem;
		Aether::System m_FrameFlushSystem;
	};
}
