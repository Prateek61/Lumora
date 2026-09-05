#pragma once

#include "Lumora/Lumen/RenderDevice.h"
#include <unordered_map>

struct GLFWwindow; // Forward declaration to avoid including GLFW headers

namespace Lumora::Lumen
{
	class GLRenderDevice : public RenderDevice
	{
	public:
		explicit GLRenderDevice(const RendererProps& props = {})
			: m_Props(props) {}
		~GLRenderDevice() override;

		// Lifecycle
		void Init(void* glfwWindowHandle, void* nativeWindowHandle) override;
		void Shutdown() override;
		void BeginFrame() override;
		void EndFrame() override;
		void OnResize(uint32_t width, uint32_t height) override;

		// State
		void SetClearColor(glm::vec4 color) override;
		void Clear() override;
		void SetViewport(glm::vec<2, uint32_t> pos, glm::vec<2, uint32_t> size) override;

		// Buffers
		BufferHandle CreateVertexBuffer(const void* data, uint32_t size, const VertexLayout& layout, bool dynamic = false) override;
		void UpdateVertexBuffer(BufferHandle buffer, const void* data, uint32_t size, uint32_t offset) override;
		BufferHandle CreateIndexBuffer(const void* data, uint32_t count) override;
		BufferHandle CreateUniformBuffer(uint32_t size) override;
		void UpdateUniformBuffer(BufferHandle buffer, const void* data, uint32_t size, uint32_t offset) override;
		void DestroyBuffer(BufferHandle buffer) override;

		// Shaders
		ShaderHandle CreateShader(const char* vertexSource, const char* fragmentSource) override;
		ShaderHandle CreateComputeShader(const char* computeSource) override;
		void DestroyShader(ShaderHandle shader) override;
		void BindShader(ShaderHandle shader) override;

		// Uniforms
		void BindUniformBuffer(BufferHandle buffer, uint32_t slot) override;

		// Textures
		TextureHandle CreateTexture2D(uint32_t width, uint32_t height, const void* pixelData, uint32_t channels = 4u) override;
		void BindTexture(TextureHandle texture, uint32_t slot) override;
		void DestroyTexture(TextureHandle texture) override;

		// Drawing
		void DrawIndexed(BufferHandle vertexBuffer, BufferHandle indexBuffer, uint32_t indexCount) override;
		void DispatchCompute(ShaderHandle computeShader, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) override;

		// Info 
		RenderAPI GetAPI() const override { return RenderAPI::OpenGL; }

	private:
		GLFWwindow* m_GLFWWindowHandle = nullptr;
		RendererProps m_Props;

		// Internal Resource Pool
		uint32_t m_NextHandleId = 1;

		struct GLVertexBuffer
		{
			uint32_t VAO = 0;
			uint32_t VBO = 0;
			VertexLayout Layout;
		};
		struct GLIndexBuffer
		{
			uint32_t IBO = 0;
			uint32_t Count = 0;
		};
		struct GLUniformBuffer
		{
			uint32_t BufferID = 0;
			uint32_t Size = 0;
		};

		std::unordered_map<uint32_t, GLVertexBuffer> m_VertexBuffers;
		std::unordered_map<uint32_t, GLIndexBuffer> m_IndexBuffers;
		std::unordered_map<uint32_t, uint32_t> m_Shaders; // ShaderHandle.Id -> GL Program ID
		std::unordered_map<uint32_t, uint32_t> m_Textures; // TextureHandle.Id -> GL Texture ID
		std::unordered_map<uint32_t, GLUniformBuffer> m_UniformBuffers; // BufferHandle.Id -> GL Uniform Buffer

		uint32_t AllocHandle() { return m_NextHandleId++; }

		// Helpers
		static uint32_t CompileShaderStage(uint32_t type, const char* source);
		static uint32_t LinkShaderProgram(uint32_t vertexShader, uint32_t fragmentShader);
		static void SetupVertexAttributes(const VertexLayout& layout);
	};
}