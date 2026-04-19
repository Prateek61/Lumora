#include "LMPCH.h"

#include "OpenGLImGuiBackend.h"
#include "Lumora/Flux/Window.h"

#include <GLFW/glfw3.h>
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

namespace Lumora::Glyph
{
	void OpenGLImGuiBackend::Init(Flux::Window& window)
	{
		LM_PROFILE_FUNCTION();
		IMGUI_CHECKVERSION();

		m_GLFWWindow = static_cast<GLFWwindow*>(window.GetGLFWHandle());
		LM_CORE_ASSERT(m_GLFWWindow, "ImGui OpenGL backend: Flux::Window has no GLFW handle!");

	
		const bool platform_ok = ImGui_ImplGlfw_InitForOpenGL(m_GLFWWindow, true);
		LM_CORE_ASSERT(platform_ok, "Failed to initialize ImGui GLFW platform backend!");

		const bool renderer_ok = ImGui_ImplOpenGL3_Init("#version 460");
		LM_CORE_ASSERT(renderer_ok, "Failed to initialize ImGui OpenGL3 backend!");
	}

	void OpenGLImGuiBackend::Shutdown()
	{
		LM_PROFILE_FUNCTION();
		// Reverse of Init: renderer first, then platform.
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		m_GLFWWindow = nullptr;
	}

	void OpenGLImGuiBackend::NewFrame()
	{
		LM_PROFILE_FUNCTION();

		ImGui_ImplGlfw_NewFrame();
		ImGui_ImplOpenGL3_NewFrame();
	}

	void OpenGLImGuiBackend::RenderDrawData(ImDrawData* draw_data)
	{
		LM_PROFILE_FUNCTION();
		ImGui_ImplOpenGL3_RenderDrawData(draw_data);
	}

	void OpenGLImGuiBackend::UpdateAndRenderPlatformWindows()
	{
		LM_PROFILE_FUNCTION();
		
		GLFWwindow* const backup_context = glfwGetCurrentContext();
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
		glfwMakeContextCurrent(backup_context);
	}
}
