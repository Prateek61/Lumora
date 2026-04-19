#include "LMPCH.h"
#include "RendererPlugin.h"

#include "Lumora/Flux/WindowPlugin.h"
#include "Lumora/Flux/Window.h"
#include "Lumora/Core/Application.h"
#

#include <GLFW/glfw3.h>

namespace
{
	void BeginFrame(const Lumora::Aether::QueryRes& res)
	{
		LM_PROFILE_FUNCTION();

		auto& deviceResource = res.World().GetResourceMut<Lumora::Lumen::RenderDeviceResource>();
		LM_CORE_ASSERT(deviceResource.Resource != nullptr, "RenderDeviceResource is null");

		{
			LM_PROFILE_SCOPE("RenderDevice::BeginFrame");
			deviceResource.Resource->BeginFrame();
		}
	}

	void PresentFrame(const Lumora::Aether::QueryRes& res)
	{
		LM_PROFILE_FUNCTION();

		auto& deviceResource = res.World().GetResourceMut<Lumora::Lumen::RenderDeviceResource>();
		LM_CORE_ASSERT(deviceResource.Resource != nullptr, "RenderDeviceResource is null");
		{
			LM_PROFILE_SCOPE("RenderDevice::EndFrame");
			deviceResource.Resource->EndFrame();
		}
	}
}

namespace Lumora::Lumen
{
	void RendererPlugin::Build(Core::Application& app)
	{
		LM_PROFILE_FUNCTION();

		// Create the RenderAPI and insert it as a resource
		auto device = RenderDevice::Create(m_Props);
		// Initialize
		auto& windowRes = app.GetWorld().GetResourceMut<Flux::WindowResource>();
		device->Init(windowRes.Resource->GetGLFWHandle(), windowRes.Resource->GetNativeHandle());
		device->SetClearColor({0.1f, 0.1f, 0.1f, 1.0f});

		// Insert the RenderDevice as a resource for systems to use
		app.InsertResource(RenderDeviceResource{std::move(device)});

		// Register a system for BeginFrame and EndFrame
		auto begin_frame_system_builder = app.GetWorld().System("Renderer::BeginFrame");
		begin_frame_system_builder.Write<RenderDeviceResource>().SetPhase<Aether::Phases::PreRender>();
		m_BeginFrameSystem = begin_frame_system_builder.Run(BeginFrame);

		auto present_frame_system_builder = app.GetWorld().System("Renderer::PresentFrame");
		present_frame_system_builder.Write<RenderDeviceResource>().SetPhase<Aether::Phases::Present>();
		m_PresentFrameSystem = present_frame_system_builder.Run(PresentFrame);
	}

	void RendererPlugin::Finish(Core::Application& app) {}

	void RendererPlugin::Cleanup(Core::Application& app)
	{
		LM_PROFILE_FUNCTION();

		// Cleanup the RenderDevice
		auto& deviceRes = app.GetWorld().GetResourceMut<RenderDeviceResource>();
		deviceRes.Resource.reset();
	}

	void RendererPlugin::AddDependencies(Core::DependencyList& dependencies)
	{
		dependencies.Require<Flux::WindowPlugin>();
	}
}
