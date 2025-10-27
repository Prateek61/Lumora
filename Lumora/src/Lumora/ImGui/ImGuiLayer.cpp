#include "LMPCH.h"
#include "ImGuiLayer.h"

#include "Lumora/Core/Application.h"
#include "Lumora/Event/Event.h"
#include "Lumora/Asset/Assets.h"
#include "Lumora/ImGui/ImGuiContext.h"

#include "imgui_internal.h"

#include "backends/imgui_impl_glfw.h"
#include "GLFW/glfw3.h"


namespace
{
	ImGuiWindowFlags DockSpaceWindowFlags;
	ImGuiDockNodeFlags DockSpaceFlags;
}

namespace Lumora
{
	ImGuiLayer::ImGuiLayer()
	{
		LM_PROFILE_FUNCTION();
	}

	ImGuiLayer::~ImGuiLayer()
	{
		LM_PROFILE_FUNCTION();
	}

	void ImGuiLayer::OnAttach()
	{
		LM_PROFILE_FUNCTION();

		IMGUI_CHECKVERSION();

		auto& app = Application::Get();
		auto& window = app.GetWindow();

		LumoraImGuiContext::Get().Init();

		ImGuiIO& io = ImGui::GetIO();
		(void)io;
		io.DisplaySize = ImVec2(static_cast<float>(window.GetWidth()), static_cast<float>(window.GetHeight()));

		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport / Platform Windows

		m_IniPath = Assets::GetFullAssetPath("ImGui.local.ini").string();
		io.IniFilename = m_IniPath.c_str();
		LM_CORE_TRACE("ImGui ini path: {0}", io.IniFilename);

		// Style
		SetStyle(true);

		io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset | ImGuiBackendFlags_RendererHasTextures;
		io.ConfigDebugHighlightIdConflicts = false;
		DEBUG_ONLY
		(
			io.ConfigDebugHighlightIdConflicts = true;
		)

		ImGui_ImplGlfw_InitForOther(static_cast<GLFWwindow*>(window.GetGLFWWindow()), true);

		InitializeDockSpace();
		// TODO: ImGuizmo
	}

	void ImGuiLayer::OnDetach()
	{
		LM_PROFILE_FUNCTION();

		// TODO: ImGuizmo

		ShutDownDockSpace();

		ImGui_ImplGlfw_Shutdown();

		LumoraImGuiContext::Get().Shutdown();
	}

	void ImGuiLayer::OnEvent(Event& e)
	{
		LM_PROFILE_FUNCTION();

		ImGuiIO& io = ImGui::GetIO();

		if (m_Block)
		{
			e.Handled |= e.IsInCategory(EventCategoryMouse) && io.WantCaptureMouse;
			e.Handled |= e.IsInCategory(EventCategoryKeyboard) && io.WantCaptureKeyboard;
		}
	}

	void ImGuiLayer::OnImGuiRender(TimeStep ts)
	{
		LM_PROFILE_FUNCTION();

		ImGuiIO& io = ImGui::GetIO();
		float del_time = ts.GetSeconds();
		io.DeltaTime = del_time > 0.0f ? del_time : (1.0f / 60.0f);

		static bool show = true;
		ImGui::ShowDemoWindow(&show);
	}

	void ImGuiLayer::BeginImGuiFrame()
	{
		LM_PROFILE_FUNCTION();

		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		BeginDockSpace();
		// TODO: ImGuizmo
	}

	void ImGuiLayer::EndImGuiFrame()
	{
		LM_PROFILE_FUNCTION();

		EndDockSpace();
		ImGui::Render();

		auto& ctx = LumoraImGuiContext::Get();
		ctx.Render(ImGui::GetDrawData(), ctx.MainView);

		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}
	}

	void ImGuiLayer::InitializeDockSpace()
	{
		LM_PROFILE_FUNCTION();

		// Set up window flags that don't need to change per frame
		DockSpaceWindowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		DockSpaceWindowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize
			| ImGuiWindowFlags_NoMove;
		DockSpaceWindowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

		DockSpaceFlags = ImGuiDockNodeFlags_None;
		// Enable passthru dockspace
		DockSpaceFlags |= ImGuiDockNodeFlags_PassthruCentralNode;
		if (DockSpaceFlags & ImGuiDockNodeFlags_PassthruCentralNode)
			DockSpaceWindowFlags |=
				ImGuiWindowFlags_NoBackground;
	}

	void ImGuiLayer::ShutDownDockSpace()
	{
		LM_PROFILE_FUNCTION();
	}

	void ImGuiLayer::BeginDockSpace()
	{
		LM_PROFILE_FUNCTION();

		ImGui::DockSpaceOverViewport(0, nullptr, DockSpaceFlags);
		return;

		static bool dockspace_open = true;

		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

		ImGui::Begin("DockSpace", &dockspace_open, DockSpaceWindowFlags);
		ImGui::PopStyleVar(3);

		// Submit the DockSpace
		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), DockSpaceFlags);
		}
	}

	void ImGuiLayer::EndDockSpace()
	{
		LM_PROFILE_FUNCTION();

		//ImGui::End();
	}

	void ImGuiLayer::SetStyle(bool dark)
	{
		LM_PROFILE_FUNCTION();

		ImGuiStyle& style = ImGui::GetStyle();
		if (dark)
		{
			ImGui::StyleColorsDark(&style);
		}
		else
		{
			ImGui::StyleColorsLight(&style);
		}

		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}
	}

	uint32_t ImGuiLayer::GetActiveWidgetId()
	{
		return GImGui->ActiveId;
	}
}
