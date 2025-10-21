#include "LumoraLayer.h"

#include "bgfx/bgfx.h"

void LumoraLayer::OnUpdate(Lumora::TimeStep ts)
{
	m_FPS = 1.0f / ts.GetSeconds();
	bgfx::dbgTextClear();
	bgfx::dbgTextPrintf(0, 0, 0x0f, "FPS: %f", m_FPS);
	bgfx::dbgTextPrintf(0, 1, 0x0f, "ExampleAsset Data: %s", Handle.Get()->Data.c_str());
}

void LumoraLayer::OnRender()
{
}
