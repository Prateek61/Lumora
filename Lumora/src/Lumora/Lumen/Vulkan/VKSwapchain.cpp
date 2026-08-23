#include "LMPCH.h"
#include "VKSwapchain.h"

namespace Lumora::Lumen
{
	void VKSwapchain::Init(VKContext& context, uint32_t width, uint32_t height, bool vsync)
	{
		LM_PROFILE_FUNCTION();

		m_Context = &context;
		m_Vsync = vsync;

		const VkSurfaceFormatKHR surface_format = PickColorFormat();
		m_ColorFormat = surface_format.format;
		m_ColorSpace = surface_format.colorSpace;
		m_DepthFormat = PickDepthFormat();

		// Once, and never again: pipelines are built against this render pass and a resize must not invalidate them.
		CreateRenderPass();

		CreateSwapchain(width, height, VK_NULL_HANDLE);
		CreateImageViews();
		CreateDepthResources();
		CreateFramebuffers();

		LM_CORE_TRACE("Vulkan swapchain: {}x{}, {} images, {}", m_Extent.width, m_Extent.height, GetImageCount(),
		              m_Vsync ? "vsync" : "no vsync");
	}

	VkSurfaceFormatKHR VKSwapchain::PickColorFormat() const
	{
		LM_PROFILE_FUNCTION();

		uint32_t count = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(m_Context->GetPhysicalDevice(), m_Context->GetSurface(), &count, nullptr);
		std::vector<VkSurfaceFormatKHR> formats(count);
		vkGetPhysicalDeviceSurfaceFormatsKHR(m_Context->GetPhysicalDevice(), m_Context->GetSurface(), &count, formats.data());

		// UNORM on purpose, not SRGB. The GL backend draws to the default framebuffer with GL_FRAMEBUFFER_SRGB off,
		// so it writes colour values raw. An SRGB swapchain would gamma encode them and Vulkan would look washed out
		// sitting next to GL.
		for (const auto& format : formats)
		{
			if (format.format == VK_FORMAT_B8G8R8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
				return format;
		}
		for (const auto& format : formats)
		{
			if (format.format == VK_FORMAT_R8G8B8A8_UNORM)
				return format;
		}

		// Format and colour space are advertised as a pair, so both come from one entry.
		LM_CORE_WARN("No UNORM surface format available, colours will not match the OpenGL backend");
		return formats.empty() ? VkSurfaceFormatKHR{VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR} : formats[0];
	}

	VkFormat VKSwapchain::PickDepthFormat() const
	{
		LM_PROFILE_FUNCTION();

		const VkFormat candidates[] = {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT};

		for (VkFormat format : candidates)
		{
			VkFormatProperties properties{};
			vkGetPhysicalDeviceFormatProperties(m_Context->GetPhysicalDevice(), format, &properties);
			if (properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
				return format;
		}

		LM_CORE_ASSERT(false, "No usable depth format");
		return VK_FORMAT_D32_SFLOAT;
	}

	VkPresentModeKHR VKSwapchain::PickPresentMode() const
	{
		LM_PROFILE_FUNCTION();

		// FIFO is the only mode guaranteed to exist, and it is what vsync means.
		if (m_Vsync)
			return VK_PRESENT_MODE_FIFO_KHR;

		uint32_t count = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(m_Context->GetPhysicalDevice(), m_Context->GetSurface(), &count, nullptr);
		std::vector<VkPresentModeKHR> modes(count);
		vkGetPhysicalDeviceSurfacePresentModesKHR(m_Context->GetPhysicalDevice(), m_Context->GetSurface(), &count, modes.data());

		for (VkPresentModeKHR mode : modes)
		{
			if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
				return mode;
		}
		for (VkPresentModeKHR mode : modes)
		{
			if (mode == VK_PRESENT_MODE_IMMEDIATE_KHR)
				return mode;
		}
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	void VKSwapchain::CreateSwapchain(uint32_t width, uint32_t height, VkSwapchainKHR oldSwapchain)
	{
		LM_PROFILE_FUNCTION();

		VkSurfaceCapabilitiesKHR capabilities{};
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_Context->GetPhysicalDevice(), m_Context->GetSurface(), &capabilities);

		// A driver that pins the extent reports it here; otherwise clamp what the window asked for.
		if (capabilities.currentExtent.width != UINT32_MAX)
		{
			m_Extent = capabilities.currentExtent;
		}
		else
		{
			m_Extent.width = std::clamp(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			m_Extent.height = std::clamp(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
		}

		uint32_t image_count = capabilities.minImageCount + 1;
		if (capabilities.maxImageCount > 0 && image_count > capabilities.maxImageCount)
			image_count = capabilities.maxImageCount;

		VkSwapchainCreateInfoKHR create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		create_info.surface = m_Context->GetSurface();
		create_info.minImageCount = image_count;
		create_info.imageFormat = m_ColorFormat;
		create_info.imageColorSpace = m_ColorSpace;
		create_info.imageExtent = m_Extent;
		create_info.imageArrayLayers = 1;
		create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		create_info.preTransform = capabilities.currentTransform;
		create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		create_info.presentMode = PickPresentMode();
		create_info.clipped = VK_TRUE;
		create_info.oldSwapchain = oldSwapchain;

		const uint32_t families[] = {m_Context->GetGraphicsFamily(), m_Context->GetPresentFamily()};
		if (families[0] != families[1])
		{
			create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			create_info.queueFamilyIndexCount = 2;
			create_info.pQueueFamilyIndices = families;
		}
		else
		{
			create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}

		LM_VK_CHECK(vkCreateSwapchainKHR(m_Context->GetDevice(), &create_info, nullptr, &m_Swapchain));

		uint32_t actual_count = 0;
		vkGetSwapchainImagesKHR(m_Context->GetDevice(), m_Swapchain, &actual_count, nullptr);
		m_Images.resize(actual_count);
		vkGetSwapchainImagesKHR(m_Context->GetDevice(), m_Swapchain, &actual_count, m_Images.data());
	}

	void VKSwapchain::CreateImageViews()
	{
		LM_PROFILE_FUNCTION();

		m_ImageViews.resize(m_Images.size());

		for (size_t i = 0; i < m_Images.size(); ++i)
		{
			VkImageViewCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			info.image = m_Images[i];
			info.viewType = VK_IMAGE_VIEW_TYPE_2D;
			info.format = m_ColorFormat;
			info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			info.subresourceRange.layerCount = 1;
			info.subresourceRange.levelCount = 1;

			LM_VK_CHECK(vkCreateImageView(m_Context->GetDevice(), &info, nullptr, &m_ImageViews[i]));
		}
	}

	void VKSwapchain::CreateDepthResources()
	{
		LM_PROFILE_FUNCTION();

		VkImageCreateInfo image_info{};
		image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		image_info.imageType = VK_IMAGE_TYPE_2D;
		image_info.format = m_DepthFormat;
		image_info.extent = VkExtent3D{m_Extent.width, m_Extent.height, 1};
		image_info.mipLevels = 1;
		image_info.arrayLayers = 1;
		image_info.samples = VK_SAMPLE_COUNT_1_BIT;
		image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
		image_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		LM_VK_CHECK(vkCreateImage(m_Context->GetDevice(), &image_info, nullptr, &m_DepthImage));

		VkMemoryRequirements requirements{};
		vkGetImageMemoryRequirements(m_Context->GetDevice(), m_DepthImage, &requirements);

		VkMemoryAllocateInfo allocate_info {};
		allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocate_info.allocationSize = requirements.size;
		allocate_info.memoryTypeIndex = m_Context->FindMemoryType(requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		LM_VK_CHECK(vkAllocateMemory(m_Context->GetDevice(), &allocate_info, nullptr, &m_DepthMemory));
		LM_VK_CHECK(vkBindImageMemory(m_Context->GetDevice(), m_DepthImage, m_DepthMemory, 0));

		VkImageViewCreateInfo view_info{};
		view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		view_info.image = m_DepthImage;
		view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		view_info.format = m_DepthFormat;
		view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		view_info.subresourceRange.layerCount = 1;
		view_info.subresourceRange.levelCount = 1;

		LM_VK_CHECK(vkCreateImageView(m_Context->GetDevice(), &view_info, nullptr, &m_DepthView));
	}

	void VKSwapchain::CreateRenderPass()
	{
		LM_PROFILE_FUNCTION();

		VkAttachmentDescription color{};
		color.format = m_ColorFormat;
		color.samples = VK_SAMPLE_COUNT_1_BIT;
		color.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // This is where the clear colour is applied.
		color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		color.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		color.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		color.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		color.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentDescription depth{};
		depth.format = m_DepthFormat;
		depth.samples = VK_SAMPLE_COUNT_1_BIT;
		depth.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depth.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; // Nothing reads depth after the frame
		depth.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depth.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depth.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depth.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference color_ref{};
		color_ref.attachment = 0;
		color_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depth_ref{};
		depth_ref.attachment = 1;
		depth_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &color_ref;
		subpass.pDepthStencilAttachment = &depth_ref;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = dependency.srcStageMask;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		const VkAttachmentDescription attachments[] = {color, depth};

		VkRenderPassCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		info.attachmentCount = 2;
		info.pAttachments = attachments;
		info.subpassCount = 1;
		info.pSubpasses = &subpass;
		info.dependencyCount = 1;
		info.pDependencies = &dependency;

		LM_VK_CHECK(vkCreateRenderPass(m_Context->GetDevice(), &info, nullptr, &m_RenderPass));
	}

	void VKSwapchain::CreateFramebuffers()
	{
		LM_PROFILE_FUNCTION();

		m_Framebuffers.resize(m_ImageViews.size());

		for (size_t i = 0; i < m_ImageViews.size(); ++i)
		{
			const VkImageView attachments[] = {m_ImageViews[i], m_DepthView};

			VkFramebufferCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			info.renderPass = m_RenderPass;
			info.attachmentCount = 2;
			info.pAttachments = attachments;
			info.width = m_Extent.width;
			info.height = m_Extent.height;
			info.layers = 1;

			LM_VK_CHECK(vkCreateFramebuffer(m_Context->GetDevice(), &info, nullptr, &m_Framebuffers[i]));
		}
	}

	void VKSwapchain::Recreate(uint32_t width, uint32_t height)
	{
		LM_PROFILE_FUNCTION();

		// Caller is responsible for having waited on device.
		DestroyImageResources();

		VkSwapchainKHR retired = m_Swapchain;
		CreateSwapchain(width, height, retired);
		CreateImageViews();
		CreateDepthResources();
		CreateFramebuffers();

		if (retired != VK_NULL_HANDLE)
			vkDestroySwapchainKHR(m_Context->GetDevice(), retired, nullptr);

		LM_CORE_TRACE("Vulkan swapchain rebuilt: {}x{}, {} images", m_Extent.width, m_Extent.height, GetImageCount());
	}

	void VKSwapchain::DestroyImageResources()
	{
		LM_PROFILE_FUNCTION();

		VkDevice device = m_Context->GetDevice();

		for (VkFramebuffer framebuffer : m_Framebuffers)
			vkDestroyFramebuffer(device, framebuffer, nullptr);
		m_Framebuffers.clear();

		if (m_DepthView != VK_NULL_HANDLE)
			vkDestroyImageView(device, m_DepthView, nullptr);
		if (m_DepthImage != VK_NULL_HANDLE)
			vkDestroyImage(device, m_DepthImage, nullptr);
		if (m_DepthMemory != VK_NULL_HANDLE)
			vkFreeMemory(device, m_DepthMemory, nullptr);
		m_DepthView = VK_NULL_HANDLE;
		m_DepthImage = VK_NULL_HANDLE;
		m_DepthMemory = VK_NULL_HANDLE;

		for (VkImageView view : m_ImageViews)
			vkDestroyImageView(device, view, nullptr);
		m_ImageViews.clear();
		m_Images.clear();
	}

	void VKSwapchain::Shutdown()
	{
		LM_PROFILE_FUNCTION();

		if (!m_Context || !m_Context->IsValid())
			return;

		DestroyImageResources();

		if (m_Swapchain != VK_NULL_HANDLE)
		{
			vkDestroySwapchainKHR(m_Context->GetDevice(), m_Swapchain, nullptr);
			m_Swapchain = VK_NULL_HANDLE;
		}

		if (m_RenderPass != VK_NULL_HANDLE)
		{
			vkDestroyRenderPass(m_Context->GetDevice(), m_RenderPass, nullptr);
			m_RenderPass = VK_NULL_HANDLE;
		}

		m_Context = nullptr;
	}
}
