#include "LMPCH.h"
#include "RenderDevice.h"

#include "Lumora/Lumen/OpenGL/GLRenderDevice.h"
#include "Lumora/Lumen/Vulkan/VKRenderDevice.h"

namespace Lumora::Lumen
{
	Scope<RenderDevice> Lumora::Lumen::RenderDevice::Create(RenderAPI api, const RendererProps& props)
	{
		LM_PROFILE_FUNCTION();

		switch (api)
		{
			case RenderAPI::None:
				LM_CORE_ASSERT(false, "RenderAPI::None is not supported");
				return nullptr;
			case RenderAPI::OpenGL:
				return CreateScope<GLRenderDevice>();
			case RenderAPI::Vulkan:
				return CreateScope<VKRenderDevice>();
		}
		LM_CORE_ASSERT(false, "Unknown RenderAPI");
		return nullptr;
	}
}


