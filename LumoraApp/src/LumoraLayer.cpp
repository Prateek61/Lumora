#include "LumoraLayer.h"

#include "bgfx/bgfx.h"

void LumoraLayer::OnUpdate(Lm::TimeStep ts)
{
	m_FPS = 1.0f / ts.GetSeconds();

	m_CumTime += ts.GetSeconds();
	float sample = (m_CumTime / 5.0f) * 3.1415f; // 5 seconds for full cycle

	float red = (std::sin(sample)) * 0.5f + 0.5f;
	float green = (std::sin(sample * 0.6f + 2.0f)) * 0.5f + 0.5f;
	float blue = (std::sin(sample * 1.6f + 4.0f)) * 0.5f + 0.5f;

	m_Color = 0x000000ff |
		(static_cast<uint32_t>(red * 255.0f) << 24) |
		(static_cast<uint32_t>(green * 255.0f) << 16) |
		(static_cast<uint32_t>(blue * 255.0f) << 8);

	Lm::Application::Get().GetRendererContext().SetClearColor(m_Color);
}

void LumoraLayer::OnRender()
{
}

void LumoraLayer::OnImGuiRender(Lumora::TimeStep ts)
{
	ImGui::Begin("Lumora Layer");
	ImGui::Text("FPS: %f", m_FPS);
	Handle && [this](const Lm::Ref<ExampleAsset>& asset)
	{
		ImGui::Text("ExampleAsset Data: %s", asset->Data.c_str());
	};
	ImGui::End();
}
