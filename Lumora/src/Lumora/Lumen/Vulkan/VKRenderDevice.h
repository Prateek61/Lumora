#pragma once

#include "Lumora/Lumen/RenderDevice.h"
#include "Lumora/Lumen/Vulkan/VKContext.h"
#include "Lumora/Lumen/Vulkan/VKSwapchain.h"
#include "Lumora/Lumen/Vulkan/VKRingAllocator.h"
#include "Lumora/Lumen/Vulkan/VKPipelineCache.h"

struct GLFWwindow;

namespace Lumora::Lumen
{
	// Everything ImGui's Vulkan backend needs
	struct VKImGuiHandoff
	{
		uint32_t ApiVersion = 0;
		VkInstance Instance = VK_NULL_HANDLE;
		VkPhysicalDevice PhysicalDevice = VK_NULL_HANDLE;
		VkDevice Device = VK_NULL_HANDLE;
		uint32_t QueueFamily = 0;
		uint32_t PresentQueueFamily = 0;
		VkQueue Queue = VK_NULL_HANDLE;
		VkRenderPass RenderPass = VK_NULL_HANDLE;
		uint32_t MinImageCount = 0;
		uint32_t ImageCount = 0;
	};

	class VKRenderDevice final : public RenderDevice
	{
	public:
		explicit VKRenderDevice(const RendererProps& props = {}) : m_Props(props) {}
		~VKRenderDevice() override;

		// Lifecycle
		void Init(void* glfwWindowHandle, void* nativeWindowHandle) override;
		void Shutdown() override;
		void BeginFrame() override;
		void EndFrame() override;
		void OnResize(uint32_t width, uint32_t height) override;

		// State
		void SetClearColor(glm::vec4 color) override;
		void Clear() override;
		void SetViewport(glm::uvec2 pos, glm::uvec2 size) override;

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
		RenderAPI GetAPI() const override { return RenderAPI::Vulkan; }
		glm::mat4 GetClipCorrection() const override;

		// The ImGui seam.
		VKImGuiHandoff GetImGuiHandoff() const;
		VkCommandBuffer GetOpenCommandBuffer() const;
		void RestoreFullViewport();
		void WaitIdle() const;

	private:
		static constexpr uint32_t MaxFramesInFlight = 2;
		static constexpr uint32_t MaxDescriptorSetsPerFrame = 64;
		static constexpr VkDeviceSize RingBlockSize = 4ull * 1024 * 1024; // 4 MB
		static constexpr VkDeviceSize DummyUniformSize = 256;

		enum class FrameState : uint8_t
		{
			Closed,   // BeginFrame has not run this tick
			Skipped,  // BeginFrame ran and declined the frame
			Recording // The Frame is open and recording commands
		};

		struct FrameData
		{
			VkCommandPool CommandPool = VK_NULL_HANDLE;
			VkCommandBuffer CommandBuffer = VK_NULL_HANDLE;
			VkSemaphore ImageAvailable = VK_NULL_HANDLE;
			VkFence InFlight = VK_NULL_HANDLE;
			VkDescriptorPool DescriptorPool = VK_NULL_HANDLE;
			VKRingAllocator Ring;
		};

		struct VKVertexBuffer
		{
			VertexLayout Layout;
			uint32_t Size = 0;
			bool Dynamic = false;

			// Static buffers own device local storage
			VkBuffer Buffer = VK_NULL_HANDLE;
			VkDeviceMemory Memory = VK_NULL_HANDLE;

			// Dynamic buffers own nothing. This is where the last update landed in the frame's ring
			VkBuffer RingBuffer = VK_NULL_HANDLE;
			VkDeviceSize RingOffset = 0;
		};

		struct VKIndexBuffer
		{
			VkBuffer Buffer = VK_NULL_HANDLE;
			VkDeviceMemory Memory = VK_NULL_HANDLE;
			uint32_t Count = 0;
		};

		struct VKUniformBuffer
		{
			uint32_t Size = 0;
			// Keep a CPU side copy
			std::vector<uint8_t> Shadow;
			VkBuffer RingBuffer = VK_NULL_HANDLE;
			VkDeviceSize RingOffset = 0;
		};

		struct VKShader
		{
			VkShaderModule Vertex = VK_NULL_HANDLE;
			VkShaderModule Fragment = VK_NULL_HANDLE;
			VkShaderModule Compute = VK_NULL_HANDLE;
		};

		struct VKTexture
		{
			VkImage Image = VK_NULL_HANDLE;
			VkDeviceMemory Memory = VK_NULL_HANDLE;
			VkImageView View = VK_NULL_HANDLE;
		};

		//Frame plumbing
		void CreateFrameData();
		void DestroyFrameData();
		void CreatePresentSemaphores();
		void DestroyPresentSemaphores();
		bool RecreateSwapchain();

		// Resource plumbing
		void CreateUploadPool();
		VkCommandBuffer BeginSingleTimeCommands();
		void EndSingleTimeCommands(VkCommandBuffer commandBuffer);
		bool CreateRawBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& outBuffer,
		                     VkDeviceMemory& outMemory) const;
		void DestroyRawBuffer(VkBuffer& buffer, VkDeviceMemory& memory) const;
		void UploadToDeviceBuffer(VkBuffer target, const void* data, VkDeviceSize size);
		bool CreateTextureResource(uint32_t width, uint32_t height, const void* rgba, VKTexture& outTexture);
		void UploadToTexture(VkImage image, uint32_t width, uint32_t height, const void* rgba);
		void DestroyTextureResource(VKTexture& texture) const;
		void DestroyShaderModules(VKShader& shader) const;
		VkShaderModule CreateShaderModule(const std::vector<uint32_t>& spirv) const;
		void CreateDefaults();
		void DestroyDefaults();

		// Frame contract and failure latch
		bool RequireOpenFrame(const char* call) const;
		bool NoteFatalResult(VkResult result);

		// Per frame and per draw
		void UploadUniform(VKUniformBuffer& uniform);
		void RefreshUniformBuffers();
		VkDescriptorSet AcquireDescriptorSet();
		void WriteDescriptorSet(VkDescriptorSet set);
		bool EnsureDescriptorSet();
		void BindDescriptorSet(VkCommandBuffer commandBuffer, VkPipelineBindPoint bindPoint) const;
		void BeginResumePass() const;

		VkCommandBuffer CurrentCommandBuffer() const { return m_Frames[m_FrameIndex].CommandBuffer; }
		VKRingAllocator& CurrentRing() { return m_Frames[m_FrameIndex].Ring; }

		RendererProps m_Props;
		GLFWwindow* m_Window = nullptr;
		VKContext m_Context;
		VKSwapchain m_Swapchain;
		VKPipelineCache m_Pipelines;

		std::array<FrameData, MaxFramesInFlight> m_Frames{};

		// Present waits on these, and a fence cannot tell you when a present in done with one. So they
		// are per swap-chain image, not per frame in flight. Getting this wrong is a validation error
		// that only shows up under load.
		std::vector<VkSemaphore> m_RenderFinished;
		std::vector<VkFence> m_ImagesInFlight; // Borrowed, not owned

		uint32_t m_FrameIndex = 0;
		uint32_t m_ImageIndex = 0;
		FrameState m_FrameState = FrameState::Closed;
		bool m_SwapchainDirty = false;

		// So that only the first run produce an error after the device is gone.
		bool m_DeviceLost = false;

		glm::vec4 m_ClearColor{0.0f, 0.0f, 0.0f, 1.0f};

		// Uploads run outside any frame, which is where Render2D::Init lives
		VkCommandPool m_UploadPool = VK_NULL_HANDLE;

		// Every texture samples the way the GL backend does, so one sampler covers all of them.
		VkSampler m_Sampler = VK_NULL_HANDLE;

		// Vulkan wants every declared binding written even when the shader never reads it
		VKTexture m_DummyTexture{};
		VkBuffer m_DummyUniform = VK_NULL_HANDLE;
		VkDeviceMemory m_DummyUniformMemory = VK_NULL_HANDLE;

		// GL's global bind points, kept by hand
		ShaderHandle m_BoundShader{};
		std::array<TextureHandle, Bindings::MaxTextureSlots> m_BoundTextures{};
		std::array<BufferHandle, Bindings::MaxUniformSlots> m_BoundUniforms{};
		VkDescriptorSet m_CurrentSet = VK_NULL_HANDLE;
		bool m_BindingsDirty = true;

		std::unordered_map<uint32_t, VKVertexBuffer> m_VertexBuffers;
		std::unordered_map<uint32_t, VKIndexBuffer> m_IndexBuffers;
		std::unordered_map<uint32_t, VKUniformBuffer> m_UniformBuffers;
		std::unordered_map<uint32_t, VKShader> m_Shaders;
		std::unordered_map<uint32_t, VKTexture> m_Textures;

		uint32_t m_NextHandleId = 1;
		uint32_t AllocHandle() { return m_NextHandleId++; }
	};
}
