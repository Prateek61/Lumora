#pragma once

#include "Lumora/Lumen/Props.h"
#include "Lumora/Lumen/RenderTypes.h"
#include "glm/glm.hpp"

namespace Lumora::Lumen
{
	class RenderDevice
	{
	public:
		virtual ~RenderDevice() = default;

		// Lifecycle
		virtual void Init(void* glfwWindowHandle, void* nativeWindowHandle) = 0;
		virtual void Shutdown() = 0;

		virtual void BeginFrame() = 0;
		virtual void EndFrame() = 0;

		virtual void OnResize(uint32_t width, uint32_t height) = 0;

		// State
		virtual void SetClearColor(glm::vec4 color) = 0;
		virtual void Clear() = 0;
		virtual void SetViewport(glm::vec<2, uint32_t> pos, glm::vec<2, uint32_t> size) = 0;

		// Vertex/Index Buffers
		virtual BufferHandle CreateVertexBuffer(const void* data, uint32_t size, const VertexLayout& layout, bool dynamic = false) = 0;
		virtual void UpdateVertexBuffer(BufferHandle buffer, const void* data, uint32_t size, uint32_t offset) = 0;
		virtual BufferHandle CreateIndexBuffer(const void* data, uint32_t count) = 0;
		virtual BufferHandle CreateUniformBuffer(uint32_t size) = 0;
		virtual void UpdateUniformBuffer(BufferHandle buffer, const void* data, uint32_t size, uint32_t offset) = 0;
		virtual void DestroyBuffer(BufferHandle buffer) = 0;

		// Shaders
		virtual ShaderHandle CreateShader(const char* vertexSource, const char* fragmentSource) = 0;
		virtual ShaderHandle CreateComputeShader(const char* computeSource) = 0;

		virtual void DestroyShader(ShaderHandle shader) = 0;
		virtual void BindShader(ShaderHandle shader) = 0;

		// Uniforms
		virtual void BindUniformBuffer(BufferHandle buffer, uint32_t slot) = 0;

		// Textures
		virtual TextureHandle CreateTexture2D(uint32_t width, uint32_t height, const void* pixelData, uint32_t channels = 4u) = 0;
		virtual void BindTexture(TextureHandle texture, uint32_t slot) = 0;
		virtual void DestroyTexture(TextureHandle texture) = 0;

		// Draw Calls
		virtual void DrawIndexed(BufferHandle vertexBuffer, BufferHandle indexBuffer, uint32_t indexCount) = 0;
		virtual void DispatchCompute(ShaderHandle computeShader, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) = 0;

		// Info
		virtual RenderAPI GetAPI() const = 0;

		// Factory
		static Scope<RenderDevice> Create(const RendererProps& props);
	};
}