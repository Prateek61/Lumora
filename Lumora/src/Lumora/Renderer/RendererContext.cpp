#include "LMPCH.h"
#include "RendererContext.h"
#include "BgfxRendererContext.h"

namespace Lumora
{
	RendererContext::API RendererContext::s_API = RendererContext::API::OPENGL;

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
		LM_CORE_ASSERT(false, "Unknown Renderer API!")
		return RendererContext::API::OPENGL;
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
		LM_CORE_ASSERT(false, "Unknown Renderer API!")
		return "Unknown";
	}

	Scope<RendererContext> RendererContext::Create()
	{
		return CreateScope<BgfxRendererContext>();
	}
}