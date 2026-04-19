#include "LMPCH.h"
#include "ImGuiPlugin.h"

#include "Lumora/Aether/System.h"
#include "Lumora/Flux/Window.h"
#include "Lumora/Flux/WindowPlugin.h"
#include "Lumora/Core/Application.h"
#include "Lumora/Lumen/RendererPlugin.h"
#include "Lumora/Glyph/ImGuiBackend.h"

#include <imgui.h>

namespace
{
	using namespace Lumora::Flux;
	using namespace Lumora;

	void ImGuiInputCapture(Aether::QueryRes& res)
	{
		LM_PROFILE_FUNCTION();

		auto world = res.World();
		auto& io_state = world.GetResourceMut<Glyph::ImGuiIOState>();
		const ImGuiIO& io = ImGui::GetIO();

		io_state.WantsCaptureMouse = io.WantCaptureMouse;
		io_state.WantsCaptureKeyboard = io.WantCaptureKeyboard;
		io_state.WantsTextInput = io.WantTextInput;
	}
}

namespace Lumora::Glyph
{
	ImGuiPlugin::ImGuiPlugin(const ImGuiSettings& settings)
		: m_InitialSettings(settings) {}

	void ImGuiPlugin::Build(Core::Application& app)
	{
		LM_PROFILE_FUNCTION();

		auto& world = app.GetWorld();

		/// 1. Create and activate the ImGui context
		m_ImGuiContext = ImGui::CreateContext();
		ImGui::SetCurrentContext(m_ImGuiContext);

		/// 2. Apply engine-side settings to ImGui IO
		ImGuiIO& io = ImGui::GetIO();
		(void)io; // Silence unused variable warning if no settings are applied
		io.IniFilename = m_InitialSettings.IniFilename;

		ImGuiConfigFlags config_flags = 0;
		if (m_InitialSettings.DockingEnabled)
			config_flags |= ImGuiConfigFlags_DockingEnable;
		if (m_InitialSettings.ViewportsEnabled)
			config_flags |= ImGuiConfigFlags_ViewportsEnable;
		io.ConfigFlags |= config_flags;

		/// 3. Expose config and state to the world
		world.SetResource(m_InitialSettings); // Read by tools/editor to know active config
		world.SetResource(ImGuiIOState{});    // Read by input system to know if ImGui wants to capture mouse/keyboard/text input

		/// 4. Platform + Renderer backend.
		const auto& window_res = world.GetResource<WindowResource>();
		const auto& render_device_res = world.GetResource<Lumen::RenderDeviceResource>();

		m_ImGuiBackend = ImGuiBackend::Create(render_device_res.Resource->GetAPI());
		m_ImGuiBackend->Init(*window_res.Resource);

		/// 5. WantCapture mirror
		auto input_system = world.System("ImGui::InputCaptureSystem");
		input_system.Write<ImGuiIOState>().SetPhase<Aether::Phases::Input>();
		input_system.Run(ImGuiInputCapture);

		/// 6. BeginFrame and EndFrame systems
		auto begin_frame_system = world.System("ImGui::BeginFrameSystem");
		begin_frame_system.SetPhase<Aether::Phases::OnUI>().Read<ImGuiSettings>();
		begin_frame_system.Run([this](Aether::QueryRes& res)
		{
			LM_PROFILE_SCOPE("ImGuiPlugin::BeginFrameSystem");

			m_ImGuiBackend->NewFrame();
			ImGui::NewFrame();

			const auto& settings = res.World().GetResource<ImGuiSettings>();
			const ImGuiIO& frame_io = ImGui::GetIO();

			if (settings.DockSpaceOverMainViewport && (frame_io.ConfigFlags & ImGuiConfigFlags_DockingEnable))
			{
				ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
			}

			if (settings.ShowDemoWindow)
				ImGui::ShowDemoWindow();
		});

		auto end_frame_system = world.System("ImGui::EndFrameSystem");
		end_frame_system.SetPhase<Aether::Phases::PostUI>();
		end_frame_system.Run([this](Aether::QueryRes&)
		{
			LM_PROFILE_SCOPE("ImGuiPlugin::EndFrameSystem");
			ImGui::Render();
			m_ImGuiBackend->RenderDrawData(ImGui::GetDrawData());

			// Multi-Viewport
			const ImGuiIO& frame_io = ImGui::GetIO();
			if (frame_io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
				m_ImGuiBackend->UpdateAndRenderPlatformWindows();
		});
	}
	void ImGuiPlugin::Finish(Core::Application&) {}
	void ImGuiPlugin::Cleanup(Core::Application& app)
	{
		LM_PROFILE_FUNCTION();

		if (m_ImGuiBackend)
		{
			m_ImGuiBackend->Shutdown();
			m_ImGuiBackend.reset();
		}
		if (m_ImGuiContext)
		{
			ImGui::DestroyContext(m_ImGuiContext);
			m_ImGuiContext = nullptr;
		}
	}
	void ImGuiPlugin::AddDependencies(Core::DependencyList& dependencies)
	{
		dependencies.Require<Flux::WindowPlugin>();
		dependencies.Require<Lumen::RendererPlugin>();
	}
}
