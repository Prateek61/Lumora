#include "LMPCH.h"

#include "VulkanImGuiBackend.h"
#include "Lumora/Flux/Window.h"
#include "Lumora/Lumen/Vulkan/VKRenderDevice.h"

#include <GLFW/glfw3.h>
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"

namespace
{
	void ReportImGuiVkResult(VkResult result)
	{
		if (result != VK_SUCCESS)
			LM_CORE_ERROR("ImGui Vulkan backend: {}", Lumora::Lumen::VkResultToString(result));
	}
}

namespace Lumora::Glyph
{
	void VulkanImGuiBackend::Init(Flux::Window& window, Lumen::RenderDevice& device)
	{
		LM_PROFILE_FUNCTION();
		IMGUI_CHECKVERSION();

		m_GLFWWindow = static_cast<GLFWwindow*>(window.GetGLFWHandle());
		LM_CORE_ASSERT(m_GLFWWindow, "ImGui Vulkan backend: Flux::Window has no GLFW handle!")

		LM_CORE_ASSERT(device.GetAPI() == Lumen::RenderAPI::Vulkan, "ImGui Vulkan backend was handed a device for another API!")
		auto* vk_device = dynamic_cast<Lumen::VKRenderDevice*>(&device);

		const Lumen::VKImGuiHandoff handoff = vk_device->GetImGuiHandoff();
		if (handoff.Device == VK_NULL_HANDLE)
		{
			LM_CORE_ERROR("ImGui Vulkan backend: the render device never came up, running without UI");
			return;
		}

		if (handoff.QueueFamily != handoff.PresentQueueFamily)
		{
			LM_CORE_WARN("Graphics and present queue families differ ({} and {}). Torn our ImGui windows may fail to present",
			             handoff.QueueFamily, handoff.PresentQueueFamily);
		}

		const bool platform_ok = ImGui_ImplGlfw_InitForVulkan(m_GLFWWindow, true);
		LM_CORE_ASSERT(platform_ok, "Failed to initialize ImGui GLFW platform backend!");

		ImGui_ImplVulkan_InitInfo info{};
		info.ApiVersion = handoff.ApiVersion;
		info.Instance = handoff.Instance;
		info.PhysicalDevice = handoff.PhysicalDevice;
		info.Device = handoff.Device;
		info.QueueFamily = handoff.QueueFamily;
		info.Queue = handoff.Queue;
		// Nonzero means ImGui creates and owns the pool.
		info.DescriptorPoolSize = DescriptorPoolSize;
		info.MinImageCount = handoff.MinImageCount;
		info.ImageCount = handoff.ImageCount;
		info.MinAllocationSize = 1024ull * 1024ull;
		info.CheckVkResultFn = ReportImGuiVkResult;

		// The main viewport draws into our frame, so it uses our render pass. Secondary viewports get
		// swapchains and passes of ImGui's own making, and only need the sample count from us.
		info.PipelineInfoMain.RenderPass = handoff.RenderPass;
		info.PipelineInfoMain.Subpass = 0;
		info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		info.PipelineInfoForViewports.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

		const bool renderer_ok = ImGui_ImplVulkan_Init(&info);
		LM_CORE_ASSERT(renderer_ok, "Failed to initialize ImGui Vulkan renderer backend!")
		if (!renderer_ok)
		{
			ImGui_ImplGlfw_Shutdown();
			return;
		}

		m_Device = vk_device;
		m_MinImageCount = handoff.MinImageCount;
	}

	void VulkanImGuiBackend::Shutdown()
	{
		LM_PROFILE_FUNCTION();

		if (m_Device == nullptr)
			return;

		// ImGui doesn't wait for the device on its own. The plugin list tears ImGui down before the device, so this is ours.
		m_Device->WaitIdle();

		// Reverse if Init: renderer first, then platform.
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();

		m_GLFWWindow = nullptr;
		m_Device = nullptr;
	}

	void VulkanImGuiBackend::NewFrame()
	{
		LM_PROFILE_FUNCTION();

		if (m_Device == nullptr)
			return;

		ImGui_ImplGlfw_NewFrame();
		ImGui_ImplVulkan_NewFrame();

		// ImGui pins its image counts at init, and its own SetMinImageCount asserts instead of
		// recovering. If a surface ever moves this, say so rather than letting it surface as a
		// swapchain that fails to build inside a torn-out window.
		const uint32_t min_image_count = m_Device->GetImGuiHandoff().MinImageCount;
		if (min_image_count != m_MinImageCount)
		{
			LM_CORE_WARN("Swapchain minImageCount moved from {} to {}. ImGui was built for the old one and cannot follow.", m_MinImageCount,
			             min_image_count);
			m_MinImageCount = min_image_count;
		}
	}

	void VulkanImGuiBackend::RenderDrawData(ImDrawData* draw_data)
	{
		LM_PROFILE_FUNCTION();

		if (m_Device == nullptr)
			return;

		VkCommandBuffer command_buffer = m_Device->GetOpenCommandBuffer();
		if (command_buffer == VK_NULL_HANDLE)
			return; // Minimized or the swapchain was stale. Nothing was recorded this tick.

		ImGui_ImplVulkan_RenderDrawData(draw_data, command_buffer);

		// ImGui leaves its last clip rect behind as the scissor. The OpenGL backend restores state on the way out, so this one does too.
		m_Device->RestoreFullViewport();
	}

	void VulkanImGuiBackend::UpdateAndRenderPlatformWindows()
	{
		LM_PROFILE_FUNCTION();

		if (m_Device == nullptr)
			return;

		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}
}
