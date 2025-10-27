#pragma once

#include "imgui.h"

#include "bgfx/bgfx.h"

namespace Lumora
{
	struct LumoraImGuiContext
	{
		ImGuiContext* ImGui{};
		bgfx::VertexLayout Layout{};
		bgfx::ProgramHandle Program{};
		bgfx::ProgramHandle ImageProgram{};
		bgfx::UniformHandle Tex{};
		bgfx::UniformHandle ImageLodEnabled{};
		bgfx::ViewId MainView{};

		void Init();
		void Shutdown();
		void Render(ImDrawData* drawData, bgfx::ViewId viewId);
		static LumoraImGuiContext& Get();
	};
}