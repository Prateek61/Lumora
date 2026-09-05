#include "LMPCH.h"
#include "ImGuiBackend.h"
#include "Lumora/Glyph/Backends/OpenGLImGuiBackend.h"
#include "Lumora/Glyph/Backends/VulkanImGuiBackend.h"

namespace Lumora::Glyph
{
	Scope<ImGuiBackend> ImGuiBackend::Create(Lumen::RenderAPI api)
	{
		switch (api)
		{
		case Lumen::RenderAPI::OpenGL: return CreateScope<OpenGLImGuiBackend>();
		case Lumen::RenderAPI::Vulkan: return CreateScope<VulkanImGuiBackend>();
		default:
			LM_CORE_ASSERT(false, "Unsupported RenderAPI for ImGui backend!")
			return nullptr;
		}
	}
}
