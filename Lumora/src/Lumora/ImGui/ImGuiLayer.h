#pragma once

#include "Lumora/Core/Layer.h"
#include <imgui.h>

namespace Lumora
{
	class ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer() override;

		void OnAttach() override;
		void OnDetach() override;
		void OnEvent(Event& e) override;
		void OnImGuiRender(TimeStep ts) override;

		void BeginImGuiFrame();
		void EndImGuiFrame();
		void InitializeDockSpace();
		void ShutDownDockSpace();
		void BeginDockSpace();
		void EndDockSpace();

		void BlockEvents(bool block) { m_Block = block; }
		void SetStyle(bool dark);
		uint32_t GetActiveWidgetId();

	private:
		bool m_Block = true;
		std::string m_IniPath;
	};
}
