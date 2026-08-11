#include "LMPCH.h"
#include "VKShaderCompiler.h"

#include <shaderc/shaderc.hpp>

namespace
{
	using Lumora::Lumen::ShaderStage;

	shaderc_shader_kind ToShadercKind(ShaderStage stage)
	{
		switch (stage)
		{
		case ShaderStage::Vertex: return shaderc_glsl_vertex_shader;
		case ShaderStage::Fragment: return shaderc_glsl_fragment_shader;
		case ShaderStage::Compute: return shaderc_glsl_compute_shader;
		}
		return shaderc_glsl_vertex_shader;
	}

	const char* StageName(ShaderStage stage)
	{
		switch (stage)
		{
		case ShaderStage::Vertex: return "Vertex";
		case ShaderStage::Fragment: return "Fragment";
		case ShaderStage::Compute: return "Compute";
		}
		return "Unknown";
	}
}

namespace Lumora::Lumen
{
	std::vector<uint32_t> CompileGLSLToSPIRV(ShaderStage stage, const char* source)
	{
		LM_PROFILE_FUNCTION();

		if (source == nullptr)
		{
			LM_CORE_ERROR("{} shader source is null", StageName(stage));
			return {};
		}

		shaderc::Compiler compiler;
		shaderc::CompileOptions options;

		// SPIR-V 1.0 is accepted by every Vulkan device and the quad shader asks for nothing newer.
		options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_0);
		options.SetSourceLanguage(shaderc_source_language_glsl);

#ifdef LM_DEBUG
		options.SetGenerateDebugInfo();
#else
		options.SetOptimizationLevel(shaderc_optimization_level_performance);
#endif

		const shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(source, ToShadercKind(stage), StageName(stage), options);

		if (result.GetCompilationStatus() != shaderc_compilation_status_success)
		{
			LM_CORE_ERROR("{} shader compilation error\n{}", StageName(stage), result.GetErrorMessage());
			return {};
		}

		if (result.GetNumWarnings() > 0)
			LM_CORE_WARN("{} shader compilation warnings\n{}", StageName(stage), result.GetErrorMessage());

		return {result.cbegin(), result.cend()};
	}
}
