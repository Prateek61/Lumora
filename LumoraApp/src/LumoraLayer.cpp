#include "LumoraLayer.h"

#include "bgfx/bgfx.h"

void LumoraLayer::OnUpdate(Lumora::TimeStep ts)
{
	m_FPS = 1.0f / ts.GetSeconds();
	bgfx::dbgTextClear();
	bgfx::dbgTextPrintf(0, 0, 0x0f, "FPS: %f", m_FPS);
}

void LumoraLayer::OnRender()
{
}

LM_REGISTER_ASSET_TYPE("ExampleAsset", ExampleAsset, ExampleAssetProps, LoadExampleAsset);
