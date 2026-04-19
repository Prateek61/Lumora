#include "LMPCH.h"

#include "OpenGLImGuiBackend.h"
#include "Lumora/Flux/Window.h"

#include "backends/imgui_impl_opengl3.h"

namespace Lumora::Glyph
{
	void OpenGLImGuiBackend::Init(Flux::Window& window)
	{
		LM_PROFILE_FUNCTION();
		IMGUI_CHECKVERSION();

		const bool ok = ImGui_ImplOpenGL3_Init("#version 460");
		LM_CORE_ASSERT(ok, "Failed to initialize ImGui OpenGL3 backend!")
	}

	void OpenGLImGuiBackend::Shutdown()
	{
		LM_PROFILE_FUNCTION();
		ImGui_ImplOpenGL3_Shutdown();
	}

	void OpenGLImGuiBackend::NewFrame()
	{
		LM_PROFILE_FUNCTION();
		ImGui_ImplOpenGL3_NewFrame();
	}

	void OpenGLImGuiBackend::RenderDrawData(ImDrawData* draw_data)
	{
		LM_PROFILE_FUNCTION();
		ImGui_ImplOpenGL3_RenderDrawData(draw_data);
	}
}
