#include "LMPCH.h"
#include "RendererContext.h"
#include "BgfxRendererContext.h"

namespace Lumora
{
	RendererContext::API RendererContext::s_API = RendererContext::API::DEFAULT;

	RendererContext::API RendererContext::StringToAPI(std::string_view api)
	{
		if (api == "OpenGL")
			return RendererContext::API::OPENGL;
		if (api == "DirectX11")
			return RendererContext::API::DIRECTX11;
		if (api == "Vulkan")
			return RendererContext::API::VULKAN;
		if (api == "DirectX12")
			return RendererContext::API::DIRECTX12;
		if (api == "Default")
			return RendererContext::API::DEFAULT;

		LM_CORE_RENDERER_WARN("Unknown Renderer API string ({}). Falling back to Default.", api);

		return RendererContext::API::DEFAULT;
	}

	const char* RendererContext::APIToString(RendererContext::API api)
	{
		switch (api)
		{
		case RendererContext::API::OPENGL:   return "OpenGL";
		case RendererContext::API::DIRECTX11: return "DirectX11";
		case RendererContext::API::VULKAN:    return "Vulkan";
		case RendererContext::API::DIRECTX12: return "DirectX12";
		case RendererContext::API::DEFAULT:   return "Default";
		}

		LM_CORE_RENDERER_WARN("Unknown Renderer API enum. Returning 'Unknown' string.");

		return "Unknown";
	}

	Scope<RendererContext> RendererContext::Create(Window& window)
	{
		auto ptr = CreateScope<BgfxRendererContext>();
		ptr->Init(window);
		return ptr;
	}
}