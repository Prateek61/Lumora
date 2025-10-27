#include "LMPCH.h"
#include "Renderer.h"

namespace Lumora
{
	Scope<RendererContext> Renderer::m_RendererContext = nullptr;

	void Renderer::Init(Window& window)
	{
		m_RendererContext = RendererContext::Create(window);
	}

	void Renderer::SetAPI(const std::string& apiStr)
	{
		RendererContext::SetAPI(RendererContext::StringToAPI(apiStr));
	}
}