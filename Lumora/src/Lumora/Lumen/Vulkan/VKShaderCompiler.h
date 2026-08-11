#pragma once

#include <cstdint>
#include <vector>

namespace Lumora::Lumen
{
	enum class ShaderStage : uint8_t
	{
		Vertex,
		Fragment,
		Compute
	};

	// RenderDevice::CreateShader is handed GLSL, so the Vulkan backend carries a compiler. Returns an empty vector
	// on failure, with shaderc's message already through LM_CORE_ERROR.
	std::vector<uint32_t> CompileGLSLToSPIRV(ShaderStage stage, const char* source);
}
