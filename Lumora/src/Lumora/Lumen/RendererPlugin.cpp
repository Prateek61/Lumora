#include "LMPCH.h"
#include "RendererPlugin.h"

#include "Lumora/Flux/WindowPlugin.h"
#include "Lumora/Flux/Window.h"
#include "Lumora/Core/Application.h"
#include "Lumora/Core/Events.h"
#include "Lumora/Flux/Events.h"

namespace
{
	void BeginFrame(const Lumora::Aether::QueryRes& res)
	{
		LM_PROFILE_FUNCTION();

		auto& deviceResource = res.World().GetResourceMut<Lumora::Lumen::RenderDeviceResource>();
		LM_CORE_ASSERT(deviceResource.Resource != nullptr, "RenderDeviceResource is null");

		// Resize event
		auto& window_resize_event = res.World().GetResource<Lumora::Core::Events<Lumora::Flux::WindowResize>>();
		for (const auto& event : window_resize_event)
		{
			deviceResource.Resource->OnResize(event.Width, event.Height);
		}

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

		// The window already committed to an API when GLFW created it, so it is the one that decides.
		auto& window_res = app.GetWorld().GetResourceMut<Flux::WindowResource>();
		auto device = RenderDevice::Create(window_res.Resource->GetProps().API, m_Props);
		LM_CORE_ASSERT(device != nullptr, "Failed to create a RenderDevice for the window's API");

		// Initialize
		device->Init(window_res.Resource->GetGLFWHandle(), window_res.Resource->GetNativeHandle());
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
