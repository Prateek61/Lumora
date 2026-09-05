#pragma once

#include "Lumora/Lumen/Vulkan/VKContext.h"

namespace Lumora::Lumen
{
	class VKSwapchain
	{
	public:
		void Init(VKContext& context, uint32_t width, uint32_t height, bool vsync);
		void Shutdown();
		void Recreate(uint32_t width, uint32_t height);

		VkSwapchainKHR GetHandle() const { return m_Swapchain; }
		VkRenderPass GetRenderPass() const { return m_RenderPass; }
		VkRenderPass GetResumeRenderPass() const { return m_ResumeRenderPass; }
		VkFramebuffer GetFramebuffer(uint32_t index) const { return m_Framebuffers[index]; }
		VkFormat GetColorFormat() const { return m_ColorFormat; }
		VkFormat GetDepthFormat() const { return m_DepthFormat; }
		VkExtent2D GetExtent() const { return m_Extent; }
		uint32_t GetImageCount() const { return static_cast<uint32_t>(m_Images.size()); }
		uint32_t GetMinImageCount() const { return m_MinImageCount; }

	private:
		void CreateSwapchain(uint32_t width, uint32_t height, VkSwapchainKHR oldSwapchain);
		void CreateImageViews();
		void CreateDepthResources();
		void CreateRenderPasses();
		VkRenderPass BuildRenderPass(bool load) const;
		void CreateFramebuffers();
		void DestroyImageResources();

		VkSurfaceFormatKHR PickColorFormat() const;
		VkFormat PickDepthFormat() const;
		VkPresentModeKHR PickPresentMode() const;

		VKContext* m_Context = nullptr;
		bool m_Vsync = true;

		VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;
		VkRenderPass m_RenderPass = VK_NULL_HANDLE;
		VkRenderPass m_ResumeRenderPass = VK_NULL_HANDLE;
		VkFormat m_ColorFormat = VK_FORMAT_UNDEFINED;
		VkFormat m_DepthFormat = VK_FORMAT_UNDEFINED;
		VkColorSpaceKHR m_ColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
		VkExtent2D m_Extent{};

		uint32_t m_MinImageCount = 0;

		std::vector<VkImage> m_Images;
		std::vector<VkImageView> m_ImageViews;
		std::vector<VkFramebuffer> m_Framebuffers;

		VkImage m_DepthImage = VK_NULL_HANDLE;
		VkDeviceMemory m_DepthMemory = VK_NULL_HANDLE;
		VkImageView m_DepthView = VK_NULL_HANDLE;
	};
}