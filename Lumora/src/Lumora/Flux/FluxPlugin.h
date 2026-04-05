#pragma once

#include "Lumora/Core/Plugin.h"
#include "Lumora/Core/SmartPointers.h"
#include "Lumora/Flux/RawEvents.h"
#include "Lumora/Flux/WindowProps.h"

namespace Lumora::Flux
{
	class Window;

	using WindowResource = ScopedResource<Window>;

	class FluxPlugin : public Core::Plugin
	{
	public:
		FluxPlugin(WindowProps props = {});
		
		void Build(Core::Application& app) override;
		void Cleanup(Core::Application& app) override;

		const char* GetName() const override { return "WindowPlugin"; }
	private:
		Raw::RawEventBuffer m_EventBuffer;
		WindowProps m_InitialProps;
	};
}