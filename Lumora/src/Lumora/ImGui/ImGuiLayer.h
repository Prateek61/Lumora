#pragma once

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include "Lumora/Core/Layer.h"
#include <bgfx/bgfx.h>

#define IMGUI_FLAGS_NONE        UINT8_C(0x00)
#define IMGUI_FLAGS_ALPHA_BLEND UINT8_C(0x01)

namespace Lumora
{
	class ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer(float fontSize = 10.0f);
		~ImGuiLayer() override = default;

		void OnAttach() override;
		void OnDetach() override;
		void OnEvent(Event& e) override;
		void OnImGuiRender(TimeStep ts) override;

		void BeginImGuiFrame();
		void EndImGuiFrame();
		void InitializeDockSpace() {}
		void ShutDownDockSpace() {}
		void BeginDockSpace() {}
		void EndDockSpace() {}

		void BlockEvents(bool block) { m_Block = block; }
		void SetStyle(bool dark);
		uint32_t GetActiveWidgetId();

	private:
		bool m_Block = true;

	private:
		void Render(ImDrawData* drawData);

	private:
		// Bgfx Stuff
		ImGuiContext* m_ImGui;
		bgfx::VertexLayout m_Layout;
		bgfx::ProgramHandle m_Program;
		bgfx::ProgramHandle m_ImageProgram;
		bgfx::UniformHandle m_Tex;
		bgfx::UniformHandle m_ImageLodEnabled;
		int64_t m_Last;
		int32_t m_LastScroll;
		bgfx::ViewId m_ViewId;
	};
}
