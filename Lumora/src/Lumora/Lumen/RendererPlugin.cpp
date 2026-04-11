#include "LMPCH.h"
#include "RendererPlugin.h"

#include "Lumora/Flux/WindowPlugin.h"
#include "Lumora/Flux/Window.h"
#include "Lumora/Core/Application.h"

#include <GLFW/glfw3.h>

namespace Lumora::Lumen
{
	void RendererPlugin::Build(Core::Application& app)
	{
		LM_PROFILE_FUNCTION();

		// Create the RenderAPI and insert it as a resource
		auto device = RenderDevice::Create(m_Props);
		// Initialize
		auto& windowRes = app.GetWorld().GetResourceMut<Flux::WindowResource>();
		void* glfwWindowHandle = windowRes.Resource->GetGLFWHandle();
		device->Init(windowRes.Resource->GetGLFWHandle(), windowRes.Resource->GetNativeHandle());
		device->SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });

		// Insert the RenderDevice as a resource for systems to use
		app.InsertResource(RenderDeviceResource{ std::move(device) });

		// Register a system for BeginFrame and EndFrame
		auto& f_world = app.GetWorld().Raw();
		f_world.system<RenderDeviceResource>("Lumen::Renderer::BeginFrame")
			.kind(f_world.entity<Aether::Phases::PreStore>())
			.each([](RenderDeviceResource& deviceResource)
				{
					LM_CORE_ASSERT(deviceResource.Resource != nullptr, "RenderDeviceResource is null");
					
					deviceResource.Resource->BeginFrame();
				});

		f_world.system<RenderDeviceResource>("Lumen::Renderer::EndFrame")
			.kind(f_world.entity<Aether::Phases::OnStore>())
			.each([](RenderDeviceResource& deviceResource)
				{
					LM_CORE_ASSERT(deviceResource.Resource != nullptr, "RenderDeviceResource is null");

					deviceResource.Resource->EndFrame();
				});
	}
	void RendererPlugin::Finish(Core::Application& app)
	{
	}
	void RendererPlugin::Cleanup(Core::Application& app)
	{
		// Cleanup the RenderDevice
		auto& deviceRes = app.GetWorld().GetResourceMut<RenderDeviceResource>();
		deviceRes.Resource.reset();
	}
}
