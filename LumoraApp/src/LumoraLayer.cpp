#include "LumoraLayer.h"

#include "bgfx/bgfx.h"

void LumoraLayer::OnUpdate(Lm::TimeStep ts)
{
	m_FPS = 1.0f / ts.GetSeconds();
	bgfx::dbgTextClear();
	bgfx::dbgTextPrintf(0, 0, 0x0f, "FPS: %f", m_FPS);

	Handle && [](const Lm::Ref<ExampleAsset>& asset)
	{
		bgfx::dbgTextPrintf(0, 1, 0x0f, "ExampleAsset Data: %s", asset->Data.c_str());
	};
}

void LumoraLayer::OnRender()
{
}
