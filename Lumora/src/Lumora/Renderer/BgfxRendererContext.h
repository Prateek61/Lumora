#pragma once

#include "Lumora/Renderer/RendererContext.h"

#include "bgfx/bgfx.h"

namespace Lumora
{
	class BgfxRendererContext : public RendererContext
	{
	public:
		void Init(Window& window) override;
		void Shutdown() override;
		void BeginFrame() override;
		void EndFrame() override;
		void Resize(uint32_t width, uint32_t height) override;
		void SetClearColor(uint32_t rgba) override;

	private:
		bgfx::PlatformData m_PlatformData{};
	};
}