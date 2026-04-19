#pragma once

#include "Lumora/Glyph/ImGuiBackend.h"

struct GLFWwindow;

namespace Lumora::Glyph
{
	class OpenGLImGuiBackend final : public ImGuiBackend
	{
	public:
		void Init(Flux::Window& window) override;
		void Shutdown() override;

		void NewFrame() override;
		void RenderDrawData(ImDrawData* draw_data) override;
		void UpdateAndRenderPlatformWindows() override;

	private:
		GLFWwindow* m_GLFWWindow = nullptr;
	};
}
