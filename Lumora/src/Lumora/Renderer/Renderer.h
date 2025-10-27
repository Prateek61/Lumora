#pragma once

#include "Lumora/Renderer/RendererContext.h"

namespace Lumora
{
	class Application;

	class Renderer
	{
	public:
		static void Init(Window& window);

		static void Shutdown() { m_RendererContext->Shutdown(); m_RendererContext = nullptr; }
		static void Resize(uint32_t width, uint32_t height) { m_RendererContext->Resize(width, height); }
		static void SetClearColor(uint32_t rgba) { m_RendererContext->SetClearColor(rgba); }
		static RendererContext::API GetAPI() { return RendererContext::GetAPI(); }
		static void SetAPI(RendererContext::API api) { RendererContext::SetAPI(api); }
		static void SetAPI(const std::string& apiStr);

	private:
		static Scope<RendererContext> m_RendererContext;

	private:
		static void BeginFrame() { m_RendererContext->BeginFrame(); }
		static void EndFrame() { m_RendererContext->EndFrame(); }

		friend class Application;
	};
}