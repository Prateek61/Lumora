#include "LMPCH.h"
#include "VKRenderDevice.h"
#include "Lumora/Lumen/Vulkan/VKShaderCompiler.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Lumora::Lumen
{
	VKRenderDevice::~VKRenderDevice()
	{
		VKRenderDevice::Shutdown();
	}

	void VKRenderDevice::Init(void* glfwWindowHandle, void* nativeWindowHandle)
	{
		LM_PROFILE_FUNCTION();

		m_Window = static_cast<GLFWwindow*>(glfwWindowHandle);
		LM_CORE_ASSERT(m_Window, "VKRenderDevice needs the GLFW window handle");

		m_Context.Init(m_Window);
		if (!m_Context.IsValid())
			return;

		int width = 0;
		int height = 0;
		glfwGetFramebufferSize(m_Window, &width, &height);

		// TODO: read vsync from WindowProps once the device is handed more than two window handle.
		m_Swapchain.Init(m_Context, static_cast<uint32_t>(width), static_cast<uint32_t>(height), true);

		CreateFrameData();
		CreatePresentSemaphores();
		CreateUploadPool();

		// Built against the swap-chain's render pass, which Recreate deliberately leaves alone
		m_Pipelines.Init(m_Context, m_Swapchain.GetRenderPass());
		CreateDefaults();

		LM_CORE_INFO("VKRenderDevice Ready");
	}

	void VKRenderDevice::CreateFrameData()
	{
		LM_PROFILE_FUNCTION();

		VkCommandPoolCreateInfo pool_info{};
		pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		pool_info.queueFamilyIndex = m_Context.GetGraphicsFamily();
		pool_info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

		VkSemaphoreCreateInfo semaphore_info{};
		semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fence_info{};
		fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT; // So the first BeginFrame does not block.

		VkDescriptorPoolSize pool_size[2]{};
		pool_size[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		pool_size[0].descriptorCount = MaxDescriptorSetsPerFrame * Bindings::MaxTextureSlots;
		pool_size[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		pool_size[1].descriptorCount = MaxDescriptorSetsPerFrame * Bindings::MaxUniformSlots;

		VkDescriptorPoolCreateInfo descriptor_pool_info{};
		descriptor_pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptor_pool_info.maxSets = MaxDescriptorSetsPerFrame;
		descriptor_pool_info.poolSizeCount = 2;
		descriptor_pool_info.pPoolSizes = pool_size;

		for (FrameData& frame : m_Frames)
		{
			LM_VK_CHECK(vkCreateCommandPool(m_Context.GetDevice(), &pool_info, nullptr, &frame.CommandPool));

			VkCommandBufferAllocateInfo allocate_info{};
			allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocate_info.commandPool = frame.CommandPool;
			allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocate_info.commandBufferCount = 1;
			LM_VK_CHECK(vkAllocateCommandBuffers(m_Context.GetDevice(), &allocate_info, &frame.CommandBuffer));

			LM_VK_CHECK(vkCreateSemaphore(m_Context.GetDevice(), &semaphore_info, nullptr, &frame.ImageAvailable));
			LM_VK_CHECK(vkCreateFence(m_Context.GetDevice(), &fence_info, nullptr, &frame.InFlight));
			LM_VK_CHECK(vkCreateDescriptorPool(m_Context.GetDevice(), &descriptor_pool_info, nullptr, &frame.DescriptorPool));

			// Vertex and uniform data share one ring, which is why the usage flags are combined
			frame.Ring.Init(m_Context, RingBlockSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		}
	}

	void VKRenderDevice::CreatePresentSemaphores()
	{
		LM_PROFILE_FUNCTION();

		VkSemaphoreCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		m_RenderFinished.resize(m_Swapchain.GetImageCount());
		for (VkSemaphore& semaphore : m_RenderFinished)
			LM_VK_CHECK(vkCreateSemaphore(m_Context.GetDevice(), &info, nullptr, &semaphore));

		m_ImagesInFlight.assign(m_Swapchain.GetImageCount(), VK_NULL_HANDLE);
	}

	void VKRenderDevice::DestroyPresentSemaphores()
	{
		LM_PROFILE_FUNCTION();

		for (VkSemaphore semaphore : m_RenderFinished)
			vkDestroySemaphore(m_Context.GetDevice(), semaphore, nullptr);
		m_RenderFinished.clear();
		m_ImagesInFlight.clear();
	}

	void VKRenderDevice::DestroyFrameData()
	{
		LM_PROFILE_FUNCTION();

		for (FrameData& frame : m_Frames)
		{
			frame.Ring.Shutdown();

			if (frame.DescriptorPool != VK_NULL_HANDLE)
				vkDestroyDescriptorPool(m_Context.GetDevice(), frame.DescriptorPool, nullptr);
			if (frame.InFlight != VK_NULL_HANDLE)
				vkDestroyFence(m_Context.GetDevice(), frame.InFlight, nullptr);
			if (frame.ImageAvailable != VK_NULL_HANDLE)
				vkDestroySemaphore(m_Context.GetDevice(), frame.ImageAvailable, nullptr);
			if (frame.CommandPool != VK_NULL_HANDLE)
				vkDestroyCommandPool(m_Context.GetDevice(), frame.CommandPool, nullptr);
			frame = {};
		}
	}

	void VKRenderDevice::CreateUploadPool()
	{
		LM_PROFILE_FUNCTION();

		VkCommandPoolCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		info.queueFamilyIndex = m_Context.GetGraphicsFamily();
		info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
		LM_VK_CHECK(vkCreateCommandPool(m_Context.GetDevice(), &info, nullptr, &m_UploadPool));
	}

	bool VKRenderDevice::RecreateSwapchain()
	{
		LM_PROFILE_FUNCTION();

		int width = 0;
		int height = 0;
		glfwGetFramebufferSize(m_Window, &width, &height);
		if (width == 0 || height == 0)
			return false; // Minimized. Skip frames until there is something to draw into.

		if (NoteFatalResult(vkDeviceWaitIdle(m_Context.GetDevice())))
			return false;

		const auto previous_image_count = static_cast<uint32_t>(m_RenderFinished.size());
		m_Swapchain.Recreate(static_cast<uint32_t>(width), static_cast<uint32_t>(height));

		// The image count can change with the surface.
		if (m_Swapchain.GetImageCount() != previous_image_count)
		{
			DestroyPresentSemaphores();
			CreatePresentSemaphores();
		}
		else
		{
			// Same shape, but the fences they were paired with belong to a swapchain that is gone.
			std::fill(m_ImagesInFlight.begin(), m_ImagesInFlight.end(), VK_NULL_HANDLE);
		}

		m_SwapchainDirty = false;
		return true;
	}

	void VKRenderDevice::BeginFrame()
	{
		LM_PROFILE_FUNCTION();

		m_FrameState = FrameState::Skipped;

		if (!m_Context.IsValid() || m_DeviceLost)
			return;

		if (m_SwapchainDirty && !RecreateSwapchain())
			return;

		FrameData& frame = m_Frames[m_FrameIndex];
		const VkResult waited = vkWaitForFences(m_Context.GetDevice(), 1, &frame.InFlight, VK_TRUE, UINT64_MAX);
		if (waited != VK_SUCCESS)
		{
			LM_CORE_ERROR("vkWaitForFences failed: {0}", VkResultToString(waited));
			NoteFatalResult(waited);
			return;
		}

		VkResult acquired = vkAcquireNextImageKHR(m_Context.GetDevice(), m_Swapchain.GetHandle(), UINT64_MAX, frame.ImageAvailable,
		                                          VK_NULL_HANDLE, &m_ImageIndex);
		if (acquired == VK_ERROR_OUT_OF_DATE_KHR)
		{
			m_SwapchainDirty = true;
			return; // No frame this tick. EndFrame sees a non-recording state and does nothing.
		}
		if (acquired != VK_SUCCESS && acquired != VK_SUBOPTIMAL_KHR)
		{
			LM_CORE_ERROR("vkAcquireNextImageKHR failed: {0}", VkResultToString(acquired));
			if (!NoteFatalResult(acquired))
				m_SwapchainDirty = true;
			return;
		}

		// A previous frame may still be reading this image
		if (m_ImagesInFlight[m_ImageIndex] != VK_NULL_HANDLE)
		{
			LM_VK_CHECK(vkWaitForFences(m_Context.GetDevice(), 1, &m_ImagesInFlight[m_ImageIndex], VK_TRUE, UINT64_MAX));
		}
		m_ImagesInFlight[m_ImageIndex] = frame.InFlight;

		LM_VK_CHECK(vkResetFences(m_Context.GetDevice(), 1, &frame.InFlight));
		LM_VK_CHECK(vkResetCommandPool(m_Context.GetDevice(), frame.CommandPool, 0));

		// The fence above means everything this frame's ring and pool held is finished with
		LM_VK_CHECK(vkResetDescriptorPool(m_Context.GetDevice(), frame.DescriptorPool, 0));
		frame.Ring.Reset();
		m_CurrentSet = VK_NULL_HANDLE;
		m_BindingsDirty = true;
		RefreshUniformBuffers();

		VkCommandBufferBeginInfo begin_info{};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		LM_VK_CHECK(vkBeginCommandBuffer(frame.CommandBuffer, &begin_info));

		VkClearValue clear_values[2]{};
		clear_values[0].color = {{m_ClearColor.r, m_ClearColor.g, m_ClearColor.b, m_ClearColor.a}};
		clear_values[1].depthStencil = VkClearDepthStencilValue{1.0f, 0};

		VkRenderPassBeginInfo pass_info{};
		pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		pass_info.renderPass = m_Swapchain.GetRenderPass();
		pass_info.framebuffer = m_Swapchain.GetFramebuffer(m_ImageIndex);
		pass_info.renderArea.extent = m_Swapchain.GetExtent();
		pass_info.clearValueCount = 2;
		pass_info.pClearValues = clear_values;

		vkCmdBeginRenderPass(frame.CommandBuffer, &pass_info, VK_SUBPASS_CONTENTS_INLINE);

		m_FrameState = FrameState::Recording;

		const VkExtent2D extent = m_Swapchain.GetExtent();
		SetViewport({0u, 0u}, {extent.width, extent.height});
	}

	void VKRenderDevice::EndFrame()
	{
		LM_PROFILE_FUNCTION();

		if (m_FrameState != FrameState::Recording)
		{
			// BeginFrame bailed out; nothing was recorded. The tick is over either way.
			m_FrameState = FrameState::Closed;
			return;
		}

		FrameData& frame = m_Frames[m_FrameIndex];

		vkCmdEndRenderPass(frame.CommandBuffer);
		LM_VK_CHECK(vkEndCommandBuffer(frame.CommandBuffer));

		constexpr VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

		VkSubmitInfo submit{};
		submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit.waitSemaphoreCount = 1;
		submit.pWaitSemaphores = &frame.ImageAvailable;
		submit.pWaitDstStageMask = &wait_stage;
		submit.commandBufferCount = 1;
		submit.pCommandBuffers = &frame.CommandBuffer;
		submit.signalSemaphoreCount = 1;
		submit.pSignalSemaphores = &m_RenderFinished[m_ImageIndex];

		LM_VK_CHECK(vkQueueSubmit(m_Context.GetGraphicsQueue(), 1, &submit, frame.InFlight));

		VkSwapchainKHR swapchain = m_Swapchain.GetHandle();

		VkPresentInfoKHR present{};
		present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		present.waitSemaphoreCount = 1;
		present.pWaitSemaphores = &m_RenderFinished[m_ImageIndex];
		present.swapchainCount = 1;
		present.pSwapchains = &swapchain;
		present.pImageIndices = &m_ImageIndex;

		const VkResult presented = vkQueuePresentKHR(m_Context.GetPresentQueue(), &present);
		if (presented == VK_ERROR_OUT_OF_DATE_KHR || presented == VK_SUBOPTIMAL_KHR)
			m_SwapchainDirty = true;
		else if (presented != VK_SUCCESS)
		{
			LM_CORE_ERROR("vkQueuePresentKHR failed: {0}", VkResultToString(presented));
			if (!NoteFatalResult(presented))
				m_SwapchainDirty = true;
		}

		m_FrameState = FrameState::Closed;
		m_FrameIndex = (m_FrameIndex + 1) % MaxFramesInFlight;
	}

	void VKRenderDevice::OnResize(uint32_t width, uint32_t height)
	{
		LM_PROFILE_FUNCTION();

		// the rebuild happens at the top of the next BeginFrame, where nothing is half recorded. The
		// size comes from GLFW then, so the event dimensions are only a trigger.
		(void)width;
		(void)height;
		m_SwapchainDirty = true;
	}

	void VKRenderDevice::SetClearColor(glm::vec4 color)
	{
		// Consumed by the render pass load op at the next BeginFrame.
		m_ClearColor = color;
	}

	void VKRenderDevice::Clear()
	{
		LM_PROFILE_FUNCTION();

		// BeginFrame already cleared through the load op. This is only for an explicit mid-frame clear.
		if (!RequireOpenFrame("Clear"))
			return;

		VkClearAttachment attachments[2]{};
		attachments[0].aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		attachments[0].colorAttachment = 0;
		attachments[0].clearValue.color = {{m_ClearColor.r, m_ClearColor.g, m_ClearColor.b, m_ClearColor.a}};
		attachments[1].aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		attachments[1].clearValue.depthStencil = VkClearDepthStencilValue{1.0f, 0};

		const VkExtent2D extent = m_Swapchain.GetExtent();

		VkClearRect rect{};
		rect.rect.extent = extent;
		rect.layerCount = 1;

		vkCmdClearAttachments(CurrentCommandBuffer(), 2, attachments, 1, &rect);
	}

	void VKRenderDevice::SetViewport(glm::uvec2 pos, glm::uvec2 size)
	{
		LM_PROFILE_FUNCTION();

		if (!RequireOpenFrame("SetViewport"))
			return;

		// Positive height. The Y flip lives in GetClipCorrection so that Y and depth are handled by
		// one mechanism instead of two.
		VkViewport viewport{};
		viewport.x = static_cast<float>(pos.x);
		viewport.y = static_cast<float>(pos.y);
		viewport.width = static_cast<float>(size.x);
		viewport.height = static_cast<float>(size.y);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{};
		scissor.offset = VkOffset2D{static_cast<int32_t>(pos.x), static_cast<int32_t>(pos.y)};
		scissor.extent = VkExtent2D{size.x, size.y};

		vkCmdSetViewport(CurrentCommandBuffer(), 0, 1, &viewport);
		vkCmdSetScissor(CurrentCommandBuffer(), 0, 1, &scissor);
	}


	glm::mat4 VKRenderDevice::GetClipCorrection() const
	{
		// Callers author in OpenGL clip space: Y up, depth -1..1. Vulkan wants Y down, depth 0..1.
		// (x, y, z, w) -> (x, -y, 0.5z + 0.5w, w)
		glm::mat4 correction(1.0f);
		correction[1][1] = -1.0f;
		correction[2][2] = 0.5f;
		correction[3][2] = 0.5f;
		return correction;
	}

	// --------------------------------------------------------------------------------------------------------------------
	// Raw memory helpers
	// --------------------------------------------------------------------------------------------------------------------

	bool VKRenderDevice::CreateRawBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& outBuffer,
	                                     VkDeviceMemory& outMemory) const
	{
		LM_PROFILE_FUNCTION();

		VkBufferCreateInfo buffer_info{};
		buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer_info.size = size;
		buffer_info.usage = usage;
		buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		LM_VK_CHECK(vkCreateBuffer(m_Context.GetDevice(), &buffer_info, nullptr, &outBuffer));
		if (outBuffer == VK_NULL_HANDLE)
			return false;

		VkMemoryRequirements requirements{};
		vkGetBufferMemoryRequirements(m_Context.GetDevice(), outBuffer, &requirements);

		VkMemoryAllocateInfo allocate_info{};
		allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocate_info.allocationSize = requirements.size;
		allocate_info.memoryTypeIndex = m_Context.FindMemoryType(requirements.memoryTypeBits, properties);
		LM_VK_CHECK(vkAllocateMemory(m_Context.GetDevice(), &allocate_info, nullptr, &outMemory));
		if (outMemory == VK_NULL_HANDLE)
		{
			vkDestroyBuffer(m_Context.GetDevice(), outBuffer, nullptr);
			outBuffer = VK_NULL_HANDLE;
			return false;
		}

		LM_VK_CHECK(vkBindBufferMemory(m_Context.GetDevice(), outBuffer, outMemory, 0));
		return true;
	}

	void VKRenderDevice::DestroyRawBuffer(VkBuffer& buffer, VkDeviceMemory& memory) const
	{
		if (buffer != VK_NULL_HANDLE)
			vkDestroyBuffer(m_Context.GetDevice(), buffer, nullptr);
		if (memory != VK_NULL_HANDLE)
			vkFreeMemory(m_Context.GetDevice(), memory, nullptr);

		buffer = VK_NULL_HANDLE;
		memory = VK_NULL_HANDLE;
	}

	VkCommandBuffer VKRenderDevice::BeginSingleTimeCommands()
	{
		LM_PROFILE_FUNCTION();

		VkCommandBufferAllocateInfo allocate_info{};
		allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocate_info.commandPool = m_UploadPool;
		allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocate_info.commandBufferCount = 1;

		VkCommandBuffer command_buffer = VK_NULL_HANDLE;
		LM_VK_CHECK(vkAllocateCommandBuffers(m_Context.GetDevice(), &allocate_info, &command_buffer));

		VkCommandBufferBeginInfo begin_info{};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		LM_VK_CHECK(vkBeginCommandBuffer(command_buffer, &begin_info));

		return command_buffer;
	}

	void VKRenderDevice::EndSingleTimeCommands(VkCommandBuffer commandBuffer)
	{
		LM_PROFILE_FUNCTION();

		LM_VK_CHECK(vkEndCommandBuffer(commandBuffer));

		VkSubmitInfo submit{};
		submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit.commandBufferCount = 1;
		submit.pCommandBuffers = &commandBuffer;

		// Uploads all happen at load time, so a queue wait is simpler than a fence and costs nothing
		LM_VK_CHECK(vkQueueSubmit(m_Context.GetGraphicsQueue(), 1, &submit, VK_NULL_HANDLE));
		LM_VK_CHECK(vkQueueWaitIdle(m_Context.GetGraphicsQueue()));

		vkFreeCommandBuffers(m_Context.GetDevice(), m_UploadPool, 1, &commandBuffer);
	}

	void VKRenderDevice::UploadToDeviceBuffer(VkBuffer target, const void* data, VkDeviceSize size)
	{
		LM_PROFILE_FUNCTION();

		VkBuffer staging = VK_NULL_HANDLE;
		VkDeviceMemory staging_memory = VK_NULL_HANDLE;
		if (!CreateRawBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging, staging_memory))
		{
			LM_CORE_ERROR("Failed to create staging buffer for device upload");
			return;
		}

		void* mapped = nullptr;
		LM_VK_CHECK(vkMapMemory(m_Context.GetDevice(), staging_memory, 0, size, 0, &mapped));
		if (mapped != nullptr)
		{
			std::memcpy(mapped, data, size);
			vkUnmapMemory(m_Context.GetDevice(), staging_memory);

			VkCommandBuffer command_buffer = BeginSingleTimeCommands();

			VkBufferCopy region{};
			region.size = size;
			vkCmdCopyBuffer(command_buffer, staging, target, 1, &region);

			EndSingleTimeCommands(command_buffer);
		}

		DestroyRawBuffer(staging, staging_memory);
	}

	// --------------------------------------------------------------------------------------------------------------------
	// Buffers
	// --------------------------------------------------------------------------------------------------------------------

	BufferHandle VKRenderDevice::CreateVertexBuffer(const void* data, uint32_t size, const VertexLayout& layout, bool dynamic)
	{
		LM_PROFILE_FUNCTION();

		if (size == 0)
		{
			LM_CORE_ERROR("Cannot create vertex buffer of size 0");
			return {0};
		}

		VKVertexBuffer vb;
		vb.Layout = layout;
		vb.Size = size;
		vb.Dynamic = dynamic;

		// A dynamic buffer owns no storage. Each update takes a fresh range out of the frame's ring, so a second Flush in the
		// same frame never writes over what the first one is still drawing.
		if (!dynamic)
		{
			if (!CreateRawBuffer(size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			                     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vb.Buffer, vb.Memory))
				return {0};

			if (data != nullptr)
				UploadToDeviceBuffer(vb.Buffer, data, size);
		}

		const uint32_t id = AllocHandle();
		m_VertexBuffers[id] = std::move(vb);
		return {id};
	}

	void VKRenderDevice::UpdateVertexBuffer(BufferHandle buffer, const void* data, uint32_t size, uint32_t offset)
	{
		LM_PROFILE_FUNCTION();

		auto it = m_VertexBuffers.find(buffer.Id);
		if (it == m_VertexBuffers.end())
		{
			LM_CORE_ERROR("Invalid Vertex Buffer Handle: {}", buffer.Id);
			return;
		}

		VKVertexBuffer& vb = it->second;
		if (!vb.Dynamic)
		{
			LM_CORE_ERROR("Vertex Buffer {} is static. Create it with dynamic = true to update it.", buffer.Id);
			return;
		}
		if (offset != 0)
		{
			LM_CORE_ERROR("Vertex Buffer {} update at offset {}. A dynamic update replaces the whole range", buffer.Id, offset);
			return;
		}
		if (size > vb.Size)
		{
			LM_CORE_ERROR("Vertex buffer update out of bounds. Handle: {}, Size: {}, Capacity: {}", buffer.Id, size, vb.Size);
			return;
		}
		// No ring to write into, and nothing will draw this tick either.
		if (!RequireOpenFrame("UpdateVertexBuffer"))
			return;

		// 16 clears every attribute format's alignment requirements, since the GPU sees (bufferOffset attributeOffset) and the
		// layout's own offsets are already aligned.
		const VKRingAllocator::Allocation allocation = CurrentRing().Allocate(size, 16);
		if (!allocation.IsValid())
			return;

		std::memcpy(allocation.Mapped, data, size);
		vb.RingBuffer = allocation.Buffer;
		vb.RingOffset = allocation.Offset;
	}

	BufferHandle VKRenderDevice::CreateIndexBuffer(const void* data, uint32_t count)
	{
		LM_PROFILE_FUNCTION();

		if (count == 0)
		{
			LM_CORE_ERROR("Cannot create index buffer of count 0");
			return {0};
		}

		VKIndexBuffer ib;
		ib.Count = count;

		const VkDeviceSize size = static_cast<VkDeviceSize>(count) * sizeof(uint32_t);
		if (!CreateRawBuffer(size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		                     ib.Buffer, ib.Memory))
			return {0};

		if (data != nullptr)
			UploadToDeviceBuffer(ib.Buffer, data, size);

		const uint32_t id = AllocHandle();
		m_IndexBuffers[id] = ib;
		return {id};
	}

	BufferHandle VKRenderDevice::CreateUniformBuffer(uint32_t size)
	{
		LM_PROFILE_FUNCTION();

		if (size == 0)
		{
			LM_CORE_ERROR("Cannot create uniform buffer with size 0");
			return {0};
		}

		// No GPU allocation. The shadow is the buffer, and the ring is where it lands each frame.
		VKUniformBuffer ubo;
		ubo.Size = size;
		ubo.Shadow.assign(size, 0);

		const uint32_t id = AllocHandle();
		m_UniformBuffers[id] = std::move(ubo);
		return {id};
	}

	void VKRenderDevice::UpdateUniformBuffer(BufferHandle buffer, const void* data, uint32_t size, uint32_t offset)
	{
		LM_PROFILE_FUNCTION();

		auto it = m_UniformBuffers.find(buffer.Id);
		if (it == m_UniformBuffers.end())
		{
			LM_CORE_ERROR("Invalid Uniform Buffer Handle: {}", buffer.Id);
			return;
		}

		VKUniformBuffer& ubo = it->second;
		if (offset > ubo.Size || size > (ubo.Size - offset))
		{
			LM_CORE_ERROR("Uniform buffer update out of bounds. Handle: {}, Size: {}, Offset: {}, Capacity: {}", buffer.Id, size, offset,
			              ubo.Size);
			return;
		}

		std::memcpy(ubo.Shadow.data() + offset, data, size);

		// Outside a frame there is no ring to land in, and the next BeginFrame re-uploads the shadow.
		if (m_FrameState == FrameState::Recording)
			UploadUniform(ubo);
	}

	void VKRenderDevice::UploadUniform(VKUniformBuffer& uniform)
	{
		LM_PROFILE_FUNCTION();

		const VkDeviceSize alignment = m_Context.GetDeviceProperties().limits.minUniformBufferOffsetAlignment;

		const VKRingAllocator::Allocation allocation = CurrentRing().Allocate(uniform.Size, alignment);
		if (!allocation.IsValid())
			return;

		std::memcpy(allocation.Mapped, uniform.Shadow.data(), uniform.Size);
		uniform.RingBuffer = allocation.Buffer;
		uniform.RingOffset = allocation.Offset;
		m_BindingsDirty = true; // The descriptor names the ring block, so a new block is a new write.
	}

	void VKRenderDevice::RefreshUniformBuffers()
	{
		LM_PROFILE_FUNCTION();

		// Rings rotate, so the copy made two frames ago is about to be handed out again.
		for (auto& ubo : m_UniformBuffers | std::views::values)
			UploadUniform(ubo);
	}

	void VKRenderDevice::DestroyBuffer(BufferHandle buffer)
	{
		LM_PROFILE_FUNCTION();

		// The only caller of this is Renderer2D::Shutdown, so a full idle is exactly right and free. A deferred deletion
		// queue is the upgrade when resources start dying mid frame.
		vkDeviceWaitIdle(m_Context.GetDevice());

		auto vb_it = m_VertexBuffers.find(buffer.Id);
		if (vb_it != m_VertexBuffers.end())
		{
			DestroyRawBuffer(vb_it->second.Buffer, vb_it->second.Memory);
			m_VertexBuffers.erase(vb_it);
			return;
		}

		auto ib_it = m_IndexBuffers.find(buffer.Id);
		if (ib_it != m_IndexBuffers.end())
		{
			DestroyRawBuffer(ib_it->second.Buffer, ib_it->second.Memory);
			m_IndexBuffers.erase(ib_it);
			return;
		}

		auto ubo_it = m_UniformBuffers.find(buffer.Id);
		if (ubo_it != m_UniformBuffers.end())
		{
			m_UniformBuffers.erase(ubo_it); // Its storage belongs to the ring.
			return;
		}

		LM_CORE_ERROR("Invalid Buffer Handle: {}", buffer.Id);
	}

	// --------------------------------------------------------------------------------------------------------------------
	// Shaders
	// --------------------------------------------------------------------------------------------------------------------

	VkShaderModule VKRenderDevice::CreateShaderModule(const std::vector<uint32_t>& spirv) const
	{
		VkShaderModuleCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		info.codeSize = spirv.size() * sizeof(uint32_t);
		info.pCode = spirv.data();

		VkShaderModule module = VK_NULL_HANDLE;
		LM_VK_CHECK(vkCreateShaderModule(m_Context.GetDevice(), &info, nullptr, &module));
		return module;
	}

	void VKRenderDevice::DestroyShaderModules(VKShader& shader) const
	{
		if (shader.Vertex != VK_NULL_HANDLE)
			vkDestroyShaderModule(m_Context.GetDevice(), shader.Vertex, nullptr);
		if (shader.Fragment != VK_NULL_HANDLE)
			vkDestroyShaderModule(m_Context.GetDevice(), shader.Fragment, nullptr);

		shader.Vertex = VK_NULL_HANDLE;
		shader.Fragment = VK_NULL_HANDLE;
	}

	ShaderHandle VKRenderDevice::CreateShader(const char* vertexSource, const char* fragmentSource)
	{
		LM_PROFILE_FUNCTION();

		const std::vector<uint32_t> vertex_spirv = CompileGLSLToSPIRV(ShaderStage::Vertex, vertexSource);
		const std::vector<uint32_t> fragment_spirv = CompileGLSLToSPIRV(ShaderStage::Fragment, fragmentSource);
		if (vertex_spirv.empty() || fragment_spirv.empty())
			return {0};

		VKShader shader;
		shader.Vertex = CreateShaderModule(vertex_spirv);
		shader.Fragment = CreateShaderModule(fragment_spirv);
		if (shader.Vertex == VK_NULL_HANDLE || shader.Fragment == VK_NULL_HANDLE)
		{
			DestroyShaderModules(shader);
			return {0};
		}

		const uint32_t id = AllocHandle();
		m_Shaders[id] = shader;
		return {id};
	}

	ShaderHandle VKRenderDevice::CreateComputeShader(const char* computeSource)
	{
		LM_CORE_ERROR("VKRenderDevice::CreateComputeShader is not implemented");
		return {0};
	}

	void VKRenderDevice::DestroyShader(ShaderHandle shader)
	{
		LM_PROFILE_FUNCTION();

		auto it = m_Shaders.find(shader.Id);
		if (it == m_Shaders.end())
			return;

		vkDeviceWaitIdle(m_Context.GetDevice());

		// Every pipeline built for this shader holds its modules, so they go together.
		m_Pipelines.DestroyShaderPipeline(shader.Id);
		DestroyShaderModules(it->second);
		m_Shaders.erase(it);

		if (m_BoundShader.Id == shader.Id)
			m_BoundShader = {};
	}

	void VKRenderDevice::BindShader(ShaderHandle shader)
	{
		LM_PROFILE_FUNCTION();

		if (!m_Shaders.contains(shader.Id))
		{
			LM_CORE_ERROR("Invalid Shader Handle: {}", shader.Id);
		}

		// Nothing is bound yet. DrawIndexed is where the shader meets a vertex layout and becomes a pipeline, which is the whole reason
		// this interface can stay GL shaped.
		m_BoundShader = shader;
	}

	void VKRenderDevice::BindUniformBuffer(BufferHandle buffer, uint32_t slot)
	{
		LM_PROFILE_FUNCTION();

		if (slot < Bindings::Uniform0 || slot >= Bindings::Uniform0 + Bindings::MaxUniformSlots)
		{
			LM_CORE_ERROR("Uniform slot {} is outside [{}, {}). Slots below {} belong to textures.", slot, Bindings::Uniform0,
			              Bindings::Uniform0 + Bindings::MaxUniformSlots, Bindings::Uniform0);
			return;
		}
		if (!m_UniformBuffers.contains(buffer.Id))
		{
			LM_CORE_ERROR("Invalid Uniform Buffer Handle: {}", buffer.Id);
			return;
		}

		const uint32_t index = slot - Bindings::Uniform0;
		if (m_BoundUniforms[index].Id != buffer.Id)
		{
			m_BoundUniforms[index] = buffer;
			m_BindingsDirty = true;
		}
	}

	// --------------------------------------------------------------------------------------------------------------------
	// Textures
	// --------------------------------------------------------------------------------------------------------------------

	bool VKRenderDevice::CreateTextureResource(uint32_t width, uint32_t height, const void* rgba, VKTexture& outTexture)
	{
		LM_PROFILE_FUNCTION();

		VkImageCreateInfo image_info{};
		image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		image_info.imageType = VK_IMAGE_TYPE_2D;
		image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
		image_info.extent = {width, height, 1};
		image_info.mipLevels = 1;
		image_info.arrayLayers = 1;
		image_info.samples = VK_SAMPLE_COUNT_1_BIT;
		image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
		image_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		LM_VK_CHECK(vkCreateImage(m_Context.GetDevice(), &image_info, nullptr, &outTexture.Image));
		if (outTexture.Image == VK_NULL_HANDLE)
			return false;

		VkMemoryRequirements requirements{};
		vkGetImageMemoryRequirements(m_Context.GetDevice(), outTexture.Image, &requirements);

		VkMemoryAllocateInfo allocate_info{};
		allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocate_info.allocationSize = requirements.size;
		allocate_info.memoryTypeIndex = m_Context.FindMemoryType(requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		LM_VK_CHECK(vkAllocateMemory(m_Context.GetDevice(), &allocate_info, nullptr, &outTexture.Memory));
		LM_VK_CHECK(vkBindImageMemory(m_Context.GetDevice(), outTexture.Image, outTexture.Memory, 0));

		UploadToTexture(outTexture.Image, width, height, rgba);

		VkImageViewCreateInfo view_info{};
		view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		view_info.image = outTexture.Image;
		view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		view_info.format = VK_FORMAT_R8G8B8A8_UNORM;
		view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		view_info.subresourceRange.levelCount = 1;
		view_info.subresourceRange.layerCount = 1;
		LM_VK_CHECK(vkCreateImageView(m_Context.GetDevice(), &view_info, nullptr, &outTexture.View));

		return outTexture.View != VK_NULL_HANDLE;
	}

	void VKRenderDevice::UploadToTexture(VkImage image, uint32_t width, uint32_t height, const void* rgba)
	{
		LM_PROFILE_FUNCTION();

		VkBuffer staging = VK_NULL_HANDLE;
		VkDeviceMemory staging_memory = VK_NULL_HANDLE;
		const VkDeviceSize size = static_cast<VkDeviceSize>(width) * static_cast<VkDeviceSize>(height) * 4ull; // RGBA8

		if (rgba != nullptr &&
		    CreateRawBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging, staging_memory))
		{
			void* mapped = nullptr;
			LM_VK_CHECK(vkMapMemory(m_Context.GetDevice(), staging_memory, 0, size, 0, &mapped));
			if (mapped != nullptr)
			{
				std::memcpy(mapped, rgba, size);
				vkUnmapMemory(m_Context.GetDevice(), staging_memory);
			}
		}

		const bool has_data = staging != VK_NULL_HANDLE;
		VkCommandBuffer command_buffer = BeginSingleTimeCommands();

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.layerCount = 1;

		if (has_data)
		{
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0,
			                     nullptr, 1, &barrier);

			VkBufferImageCopy region{};
			region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			region.imageSubresource.layerCount = 1;
			region.imageExtent = {width, height, 1};
			vkCmdCopyBufferToImage(command_buffer, staging, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		}

		// A sampler descriptor is only legal against SHADER_READ_ONLY_OPTIMAL, so even an empty texture has to make the trip.
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		vkCmdPipelineBarrier(command_buffer, has_data ? VK_PIPELINE_STAGE_TRANSFER_BIT : VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		                     VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

		EndSingleTimeCommands(command_buffer);
		DestroyRawBuffer(staging, staging_memory);
	}

	void VKRenderDevice::DestroyTextureResource(VKTexture& texture) const
	{
		if (texture.View != VK_NULL_HANDLE)
			vkDestroyImageView(m_Context.GetDevice(), texture.View, nullptr);
		if (texture.Image != VK_NULL_HANDLE)
			vkDestroyImage(m_Context.GetDevice(), texture.Image, nullptr);
		if (texture.Memory != VK_NULL_HANDLE)
			vkFreeMemory(m_Context.GetDevice(), texture.Memory, nullptr);

		texture = {};
	}

	TextureHandle VKRenderDevice::CreateTexture2D(uint32_t width, uint32_t height, const void* pixelData, uint32_t channels)
	{
		LM_PROFILE_FUNCTION();

		if (width == 0 || height == 0)
		{
			LM_CORE_ERROR("Cannot create a {}x{} texture", width, height);
			return {0};
		}

		// R8G8B8 is optional for sampled images and most desktop drivers do not offer it, while GL_RGB8 always works.
		// Widening on the CPU keeps both backend fed from the same pixels.
		std::vector<uint8_t> widened;
		const void* source = pixelData;
		if (channels == 3 && pixelData != nullptr)
		{
			const size_t pixels = static_cast<size_t>(width) * static_cast<size_t>(height);
			widened.assign(pixels * 4, 0xFF);

			auto rgb = static_cast<const uint8_t*>(pixelData);
			for (size_t i = 0; i < pixels; ++i)
			{
				widened[i * 4 + 0] = rgb[i * 3 + 0];
				widened[i * 4 + 1] = rgb[i * 3 + 1];
				widened[i * 4 + 2] = rgb[i * 3 + 2];
			}
			source = widened.data();
		}

		VKTexture texture;
		if (!CreateTextureResource(width, height, source, texture))
		{
			DestroyTextureResource(texture);
			return {0};
		}

		const uint32_t id = AllocHandle();
		m_Textures[id] = texture;
		return {id};
	}

	void VKRenderDevice::BindTexture(TextureHandle texture, uint32_t slot)
	{
		LM_PROFILE_FUNCTION();

		if (slot < Bindings::Texture0 || slot >= Bindings::Texture0 + Bindings::MaxTextureSlots)
		{
			LM_CORE_ERROR("Texture slot {} is outside [{}, {})", slot, Bindings::Texture0, Bindings::Texture0 + Bindings::MaxTextureSlots);
			return;
		}
		if (!m_Textures.contains(texture.Id))
		{
			LM_CORE_ERROR("Invalid Texture Handle: {}", texture.Id);
			return;
		}

		const uint32_t index = slot - Bindings::Texture0;
		if (m_BoundTextures[index].Id != texture.Id)
		{
			m_BoundTextures[index] = texture;
			m_BindingsDirty = true;
		}
	}

	void VKRenderDevice::DestroyTexture(TextureHandle texture)
	{
		LM_PROFILE_FUNCTION();

		auto it = m_Textures.find(texture.Id);
		if (it == m_Textures.end())
			return;

		vkDeviceWaitIdle(m_Context.GetDevice());
		DestroyTextureResource(it->second);
		m_Textures.erase(it);
	}

	// --------------------------------------------------------------------------------------------------------------------
	// Defaults and descriptors
	// --------------------------------------------------------------------------------------------------------------------

	void VKRenderDevice::CreateDefaults()
	{
		LM_PROFILE_FUNCTION();

		// The same filtering and wrapping GLRenderDevice::Texture2D sets on every texture
		VkSamplerCreateInfo sampler_info{};
		sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		sampler_info.magFilter = VK_FILTER_LINEAR;
		sampler_info.minFilter = VK_FILTER_LINEAR;
		sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
		sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		sampler_info.maxLod = 0.0f;
		LM_VK_CHECK(vkCreateSampler(m_Context.GetDevice(), &sampler_info, nullptr, &m_Sampler));

		// Magenta because it must never reach a pixel. Anything magenta on screen means a shader read a slot nothing was bound to.
		constexpr uint8_t magenta[4] = {0xFF, 0x00, 0xFF, 0xFF};
		CreateTextureResource(1, 1, magenta, m_DummyTexture);

		if (CreateRawBuffer(DummyUniformSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_DummyUniform,
		                    m_DummyUniformMemory))
		{
			void* mapped = nullptr;
			LM_VK_CHECK(vkMapMemory(m_Context.GetDevice(), m_DummyUniformMemory, 0, DummyUniformSize, 0, &mapped));
			if (mapped != nullptr)
			{
				std::memset(mapped, 0, DummyUniformSize);
				vkUnmapMemory(m_Context.GetDevice(), m_DummyUniformMemory);
			}
		}
	}

	void VKRenderDevice::DestroyDefaults()
	{
		LM_PROFILE_FUNCTION();

		DestroyTextureResource(m_DummyTexture);
		DestroyRawBuffer(m_DummyUniform, m_DummyUniformMemory);

		if (m_Sampler != VK_NULL_HANDLE)
			vkDestroySampler(m_Context.GetDevice(), m_Sampler, nullptr);
		m_Sampler = VK_NULL_HANDLE;
	}

	bool VKRenderDevice::NoteFatalResult(VkResult result)
	{
		// Log device loss once 

		if (result != VK_ERROR_DEVICE_LOST && result != VK_ERROR_SURFACE_LOST_KHR)
			return false;

		if (!m_DeviceLost)
		{
			m_DeviceLost = true;
			LM_CORE_ERROR("Vulkan device is unusable ({}). Rendering stops for the rest of this run.", VkResultToString(result));
		}
		return true;
	}

	bool VKRenderDevice::RequireOpenFrame(const char* call) const
	{
		if (m_FrameState == FrameState::Recording)
			return true;

		if (m_FrameState == FrameState::Closed)
		{
			LM_CORE_ERROR(
			    "{} was recorded outside a frame. Its system has to run after Renderer::BeginFrame, which sits in the PreRender phase.",
			    call);
			LM_CORE_ASSERT(false, "Render call recorded outside a frame");
		}
		return false;
	}

	VkDescriptorSet VKRenderDevice::AcquireDescriptorSet()
	{
		LM_PROFILE_FUNCTION();

		VkDescriptorSetLayout layout = m_Pipelines.GetSetLayout();

		VkDescriptorSetAllocateInfo allocate_info{};
		allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocate_info.descriptorPool = m_Frames[m_FrameIndex].DescriptorPool;
		allocate_info.descriptorSetCount = 1;
		allocate_info.pSetLayouts = &layout;

		// Running the pool dry is a legal result the caller is expected to handle, not a Vulkan error, so this one does not go through
		// LM_VK_CHECK.
		VkDescriptorSet set = VK_NULL_HANDLE;
		if (vkAllocateDescriptorSets(m_Context.GetDevice(), &allocate_info, &set) != VK_SUCCESS)
		{
			LM_CORE_ERROR("Out of descriptor sets this frame (pool holds {}), skipping a draw", MaxDescriptorSetsPerFrame);
			return VK_NULL_HANDLE;
		}
		return set;
	}

	void VKRenderDevice::WriteDescriptorSet(VkDescriptorSet set)
	{
		LM_PROFILE_FUNCTION();

		std::array<VkDescriptorImageInfo, Bindings::MaxTextureSlots> images{};
		std::array<VkDescriptorBufferInfo, Bindings::MaxUniformSlots> buffers{};
		std::array<VkWriteDescriptorSet, Bindings::MaxTextureSlots + Bindings::MaxUniformSlots> writes{};

		// Every binding the layout declares has to be written, bound or not. An unwritten one is undefined reads at best a lost device at worst,
		// and the dummies exist for that reason.
		for (uint32_t i = 0; i < Bindings::MaxTextureSlots; ++i)
		{
			auto it = m_Textures.find(m_BoundTextures[i].Id);
			const VKTexture& texture = it != m_Textures.end() ? it->second : m_DummyTexture;

			images[i].sampler = m_Sampler;
			images[i].imageView = texture.View;
			images[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writes[i].dstSet = set;
			writes[i].dstBinding = Bindings::Texture0 + i;
			writes[i].descriptorCount = 1;
			writes[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			writes[i].pImageInfo = &images[i];
		}

		for (uint32_t i = 0; i < Bindings::MaxUniformSlots; ++i)
		{
			auto it = m_UniformBuffers.find(m_BoundUniforms[i].Id);
			const bool bound = it != m_UniformBuffers.end() && it->second.RingBuffer != VK_NULL_HANDLE;

			// Offest stays zero: the ring offset arrives as a dynamic offset at bind time instead
			buffers[i].buffer = bound ? it->second.RingBuffer : m_DummyUniform;
			buffers[i].offset = 0;
			buffers[i].range = bound ? it->second.Size : DummyUniformSize;

			const uint32_t write = Bindings::MaxTextureSlots + i;
			writes[write].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writes[write].dstSet = set;
			writes[write].dstBinding = Bindings::Uniform0 + i;
			writes[write].descriptorCount = 1;
			writes[write].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
			writes[write].pBufferInfo = &buffers[i];
		}

		vkUpdateDescriptorSets(m_Context.GetDevice(), static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
	}

	// --------------------------------------------------------------------------------------------------------------------
	// Drawing
	// --------------------------------------------------------------------------------------------------------------------

	void VKRenderDevice::DrawIndexed(BufferHandle vertexBuffer, BufferHandle indexBuffer, uint32_t indexCount)
	{
		LM_PROFILE_FUNCTION();

		if (indexCount == 0)
			return;
		if (!RequireOpenFrame("DrawIndexed"))
			return;

		auto vb_it = m_VertexBuffers.find(vertexBuffer.Id);
		if (vb_it == m_VertexBuffers.end())
		{
			LM_CORE_ERROR("Invalid Vertex Buffer Handle: {}", vertexBuffer.Id);
			return;
		}
		auto ib_it = m_IndexBuffers.find(indexBuffer.Id);
		if (ib_it == m_IndexBuffers.end())
		{
			LM_CORE_ERROR("Invalid Index Buffer Handle: {}", indexBuffer.Id);
			return;
		}
		auto shader_it = m_Shaders.find(m_BoundShader.Id);
		if (shader_it == m_Shaders.end())
		{
			LM_CORE_ERROR("DrawIndexed with no shadow bound");
			return;
		}

		const VKVertexBuffer& vb = vb_it->second;
		const VkBuffer vertex_handle = vb.Dynamic ? vb.RingBuffer : vb.Buffer;
		if (vertex_handle == VK_NULL_HANDLE)
		{
			LM_CORE_ERROR("Vertex Buffer {} holds no data this frame", vertexBuffer.Id);
			return;
		}

		// The layout travels with the vertex buffer, so this is the first point where the pipeline key is complete
		VkPipeline pipeline = m_Pipelines.GetOrCreate(m_BoundShader.Id, shader_it->second.Vertex, shader_it->second.Fragment, vb.Layout);
		if (pipeline == VK_NULL_HANDLE)
			return;

		if (m_BindingsDirty)
		{
			m_CurrentSet = AcquireDescriptorSet();
			if (m_CurrentSet == VK_NULL_HANDLE)
				return;
			WriteDescriptorSet(m_CurrentSet);
			m_BindingsDirty = false;
		}
		if (m_CurrentSet == VK_NULL_HANDLE)
			return;

		// One per UNIFORM_BUFFER_DYNAMIC binding, in the binding order.
		uint32_t dynamic_offsets[Bindings::MaxUniformSlots]{};
		for (uint32_t i = 0; i < Bindings::MaxUniformSlots; ++i)
		{
			auto ubo_it = m_UniformBuffers.find(m_BoundUniforms[i].Id);
			if (ubo_it != m_UniformBuffers.end() && ubo_it->second.RingBuffer != VK_NULL_HANDLE)
				dynamic_offsets[i] = static_cast<uint32_t>(ubo_it->second.RingOffset);
		}

		VkCommandBuffer command_buffer = CurrentCommandBuffer();
		const VkDeviceSize vertex_offset = vb.Dynamic ? vb.RingOffset : 0;

		vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
		vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipelines.GetPipelineLayout(), 0, 1, &m_CurrentSet,
		                        Bindings::MaxUniformSlots, dynamic_offsets);
		vkCmdBindVertexBuffers(command_buffer, 0, 1, &vertex_handle, &vertex_offset);
		vkCmdBindIndexBuffer(command_buffer, ib_it->second.Buffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(command_buffer, indexCount, 1, 0, 0, 0);
	}

	void VKRenderDevice::DispatchCompute(ShaderHandle computeShader, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
	{ LM_CORE_ERROR("VKRenderDevice::DispatchCompute not implemented");
	}

	// --------------------------------------------------------------------------------------------------------------------
	// Shutdown
	// --------------------------------------------------------------------------------------------------------------------

	void VKRenderDevice::Shutdown()
	{
		LM_PROFILE_FUNCTION();

		if (!m_Context.IsValid())
			return;

		// The app can quit between BeginFrame and Present. 
		if (m_FrameState == FrameState::Recording)
		{
			vkCmdEndRenderPass(CurrentCommandBuffer());
			LM_VK_CHECK(vkEndCommandBuffer(CurrentCommandBuffer()));
			m_FrameState = FrameState::Closed;
		}

		vkDeviceWaitIdle(m_Context.GetDevice());

		for (const FrameData& frame : m_Frames)
		{
			LM_CORE_TRACE("Vulkan ring: peak {} KB used of {} KB reserved", frame.Ring.GetHighWater() / 1024,
			              frame.Ring.GetCapacity() / 1024);
		}

		for (auto& [handle, vb] : m_VertexBuffers)
		{
			LM_CORE_WARN("Leaked Vertex Buffer Handle: {}", handle);
			DestroyRawBuffer(vb.Buffer, vb.Memory);
		}
		for (auto& [handle, ib] : m_IndexBuffers)
		{
			LM_CORE_WARN("Leaked Index Buffer Handle: {}", handle);
			DestroyRawBuffer(ib.Buffer, ib.Memory);
		}
		for (auto& [handle, shader] : m_Shaders)
		{
			LM_CORE_WARN("Leaked Shader Handle: {}", handle);
			DestroyShaderModules(shader);
		}
		for (auto& [handle, texture] : m_Textures)
		{
			LM_CORE_WARN("Leaked Texture Handle: {}", handle);
			DestroyTextureResource(texture);
		}
		for (const auto handle: m_UniformBuffers | std::views::keys)
		{
			LM_CORE_WARN("Leaked Uniform Buffer Handle: {}", handle); // Its storage belongs to the ring
		}

		m_VertexBuffers.clear();
		m_IndexBuffers.clear();
		m_UniformBuffers.clear();
		m_Shaders.clear();
		m_Textures.clear();

		DestroyDefaults();

		if (m_UploadPool != VK_NULL_HANDLE)
			vkDestroyCommandPool(m_Context.GetDevice(), m_UploadPool, nullptr);
		m_UploadPool = VK_NULL_HANDLE;

		m_Pipelines.Shutdown();
		DestroyPresentSemaphores();
		DestroyFrameData();
		m_Swapchain.Shutdown();
		m_Context.Shutdown();

		m_Window = nullptr;
		m_FrameState = FrameState::Closed;
	}
}
