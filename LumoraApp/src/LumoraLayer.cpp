#include "LumoraLayer.h"

#include "bgfx/bgfx.h"

void LumoraLayer::OnUpdate(Lm::TimeStep ts)
{
	m_FPS = 1.0f / ts.GetSeconds();
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
