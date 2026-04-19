#pragma once

#include "Lumora/Glyph/ImGuiBackend.h"

namespace Lumora::Glyph
{
	class OpenGLImGuiBackend final : public ImGuiBackend
	{
	public:
		void Init(Flux::Window& window) override;
		void Shutdown() override;

		void NewFrame() override;
		void RenderDrawData(ImDrawData* draw_data) override;
	};
}
