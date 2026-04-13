#pragma once

#include "Lumora/Aether/System.h"
#include "Lumora/Lumen/RenderAPI.h"
#include "Lumora/Lumen/Props.h"
#include "Lumora/Core/Plugin.h"
#include "Lumora/Core/SmartPointers.h"
#include "Lumora/Lumen/RenderDevice.h"

namespace Lumora::Lumen
{
	using RenderDeviceResource = ScopedResource<RenderDevice>;

	class RendererPlugin : public Core::Plugin
	{
	public:
		RendererPlugin(RendererProps props = {})
			: m_Props(props) {}

		void Build(Core::Application& app) override;
		void Finish(Core::Application& app) override;
		void Cleanup(Core::Application& app) override;
		void AddDependencies(Core::DependencyList& dependencies) override;

		const char* GetName() const override { return "RendererPlugin"; }
	private:
		RendererProps m_Props;
		Aether::System m_BeginFrameSystem;
		Aether::System m_PresentFrameSystem;
	};
}