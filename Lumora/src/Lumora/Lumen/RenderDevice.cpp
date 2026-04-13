#include "LMPCH.h"
#include "RenderDevice.h"

#include "Lumora/Lumen/OpenGL/GLRenderDevice.h"

namespace Lumora::Lumen
{
	Scope<RenderDevice> Lumora::Lumen::RenderDevice::Create(const RendererProps& props)
	{
		LM_PROFILE_FUNCTION();

		switch (props.API)
		{
			case RenderAPI::None:
				LM_CORE_ASSERT(false, "RenderAPI::None is not supported");
				return nullptr;
			case RenderAPI::OpenGL:
				return CreateScope<GLRenderDevice>();
			case RenderAPI::Vulkan:
				LM_CORE_ASSERT(false, "RenderAPI::Vulkan is not supported yet");
				return nullptr;
		}
		LM_CORE_ASSERT(false, "Unknown RenderAPI");
		return nullptr;
	}
}


