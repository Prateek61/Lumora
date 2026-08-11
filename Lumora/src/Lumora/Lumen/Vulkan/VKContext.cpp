#include "LMPCH.h"
#include "VKContext.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace
{
#ifdef LM_DEBUG
	constexpr bool s_EnableValidation = true;
#else
	constexpr bool s_EnableValidation = false;
#endif

	const std::vector<const char*> s_ValidationLayers = {"VK_LAYER_KHRONOS_validation"};

	VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT* data, void* userData)
	{ 
		switch (severity)
		{
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: LM_CORE_ERROR(data->pMessage); break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: LM_CORE_WARN(data->pMessage); break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: LM_CORE_INFO(data->pMessage); break;
		default: LM_CORE_TRACE(data->pMessage); break;
		}
		return VK_FALSE;
	}

	VkDebugUtilsMessengerCreateInfoEXT MakeDebugMessengerInfo()
	{
		VkDebugUtilsMessengerCreateInfoEXT info{};
		info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
		info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		                   VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		info.pfnUserCallback = DebugCallback;
		return info;
	}

	bool ValidationLayersAvailable()
	{
		uint32_t count = 0;
		vkEnumerateInstanceLayerProperties(&count, nullptr);
		std::vector<VkLayerProperties> layers(count);
		vkEnumerateInstanceLayerProperties(&count, layers.data());

		for (const char* validationLayer : s_ValidationLayers)
		{
			bool found = false;
			for (const auto& layer : layers)
			{
				if (std::strcmp(layer.layerName, validationLayer) == 0)
				{
					found = true;
					break;
				}
			}

			if (!found)
				return false;
		}

		return true;
	}

	bool SupportsExtension(VkPhysicalDevice device, const char* name)
	{ 
		uint32_t count = 0;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr);
		std::vector<VkExtensionProperties> extensions(count);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &count, extensions.data());

		for (const auto& extension : extensions)
		{
			if (std::strcmp(extension.extensionName, name) == 0)
				return true;
		}
		return false;
	}
}

namespace Lumora::Lumen
{
	void VKContext::Init(GLFWwindow* window)
	{ 
		LM_PROFILE_FUNCTION();

		if (!glfwVulkanSupported())
		{
			LM_CORE_ASSERT(false, "GLFW reports no Vulkan loader on this machine");
			return;
		}

		if (!CreateInstance())
			return;

		CreateDebugMessenger();

		LM_VK_CHECK(glfwCreateWindowSurface(m_Instance, window, nullptr, &m_Surface));
		if (m_Surface == VK_NULL_HANDLE)
		{
			LM_CORE_ASSERT(false, "Failed to create Vulkan surface");
			return;
		}

		if (!PickPhysicalDevice())
			return;

		CreateLogicalDevice();
	}

	bool VKContext::CreateInstance()
	{
		LM_PROFILE_FUNCTION();

		VkApplicationInfo app_info{};
		app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		app_info.pApplicationName = "LumoraApp";
		app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		app_info.pEngineName = "Lumora";
		app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		app_info.apiVersion = VK_API_VERSION_1_3;

		uint32_t glfw_extension_count = 0;
		const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
		if (!glfw_extensions)
		{
			LM_CORE_ASSERT(false, "GLFW could not report the instance extensions needed for a surface");
			return false;
		}

		std::vector<const char*> extensions = {glfw_extensions, glfw_extensions + glfw_extension_count};

		const bool validation = s_EnableValidation && ValidationLayersAvailable();
		if (s_EnableValidation && !validation)
		{
			LM_CORE_WARN("Vulkan validation layer not found. Install the Vulkan SDK layer to get diagnostics.");
		}
		m_ValidationEnabled = validation;

		if (validation)
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

		VkInstanceCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		create_info.pApplicationInfo = &app_info;
		create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		create_info.ppEnabledExtensionNames = extensions.data();

		// Chaining the messenger info here covers instance creation and destruction too, which the
		// standalone messenger cannot see
		VkDebugUtilsMessengerCreateInfoEXT debug_info = MakeDebugMessengerInfo();
		if (validation)
		{
			create_info.enabledLayerCount = static_cast<uint32_t>(s_ValidationLayers.size());
			create_info.ppEnabledLayerNames = s_ValidationLayers.data();
			create_info.pNext = &debug_info;
		}

		LM_VK_CHECK(vkCreateInstance(&create_info, nullptr, &m_Instance));
		if (m_Instance == VK_NULL_HANDLE)
		{
			LM_CORE_ASSERT(false, "Failed to create Vulkan instance");
			return false;
		}

		return true;
	}

	void VKContext::CreateDebugMessenger()
	{
		// The debug utils extension rides along with the layers, so without them this entry point
		// is not ours to call.
		if (!m_ValidationEnabled)
			return;

		auto create =
		    reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(m_Instance, "vkCreateDebugUtilsMessengerEXT"));
		if (!create)
			return;

		VkDebugUtilsMessengerCreateInfoEXT info = MakeDebugMessengerInfo();
		LM_VK_CHECK(create(m_Instance, &info, nullptr, &m_DebugMessenger));
	}

	bool VKContext::PickPhysicalDevice()
	{
		LM_PROFILE_FUNCTION();
		
		uint32_t count = 0;
		vkEnumeratePhysicalDevices(m_Instance, &count, nullptr);
		if (count == 0)
		{
			LM_CORE_ASSERT(false, "No Vulkan capable GPU found");
			return false;
		}

		std::vector<VkPhysicalDevice> devices(count);
		vkEnumeratePhysicalDevices(m_Instance, &count, devices.data());

		VkPhysicalDevice best = VK_NULL_HANDLE;
		uint32_t best_graphics = UINT32_MAX;
		uint32_t best_present = UINT32_MAX;
		bool best_is_discrete = false;

		for (VkPhysicalDevice device: devices)
		{
			if (!SupportsExtension(device, VK_KHR_SWAPCHAIN_EXTENSION_NAME))
				continue;

			uint32_t family_count = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(device, &family_count, nullptr);
			std::vector<VkQueueFamilyProperties> families(family_count);
			vkGetPhysicalDeviceQueueFamilyProperties(device, &family_count, families.data());

			uint32_t graphics = UINT32_MAX;
			uint32_t present = UINT32_MAX;
			for (uint32_t i = 0; i < family_count; i++)
			{
				if (families[i].queueCount == 0)
					continue;

				if (graphics == UINT32_MAX && (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT))
					graphics = i;

				VkBool32 supports_present = VK_FALSE;
				vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_Surface, &supports_present);
				if (present == UINT32_MAX && supports_present)
					present = i;

				if ((families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && supports_present)
				{
					graphics = i;
					present = i;
					break;
				}
			}

			if (graphics == UINT32_MAX || present == UINT32_MAX)
				continue;

			uint32_t format_count = 0;
			uint32_t mode_count = 0;
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &format_count, nullptr);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &mode_count, nullptr);
			if (format_count == 0 || mode_count == 0)
				continue;

			VkPhysicalDeviceProperties properties{};
			vkGetPhysicalDeviceProperties(device, &properties);
			const bool discrete = properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;

			if (best == VK_NULL_HANDLE || (discrete && !best_is_discrete))
			{
				best = device;
				best_graphics = graphics;
				best_present = present;
				best_is_discrete = discrete;
			}
		}

		if (best == VK_NULL_HANDLE)
		{
			LM_CORE_ASSERT(false, "No GPU with a presentable graphics queue and swapchain support");
			return false;
		}

		m_PhysicalDevice = best;
		m_GraphicsFamily = best_graphics;
		m_PresentFamily = best_present;
		vkGetPhysicalDeviceProperties(m_PhysicalDevice, &m_DeviceProperties);
		vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &m_MemoryProperties);

		const uint32_t version = m_DeviceProperties.apiVersion;
		LM_CORE_INFO("Vulkan Version: {}.{}.{}", VK_VERSION_MAJOR(version), VK_VERSION_MINOR(version), VK_VERSION_PATCH(version));
		LM_CORE_INFO("Device: {}", m_DeviceProperties.deviceName);
		LM_CORE_INFO("Queue: graphics={}, present={}", m_GraphicsFamily, m_PresentFamily);
		return true;
	}

	void VKContext::CreateLogicalDevice()
	{
		LM_PROFILE_FUNCTION();

		constexpr float priority = 1.0f;
		std::vector<VkDeviceQueueCreateInfo> queue_infos;

		VkDeviceQueueCreateInfo graphics_info{};
		graphics_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		graphics_info.queueFamilyIndex = m_GraphicsFamily;
		graphics_info.queueCount = 1;
		graphics_info.pQueuePriorities = &priority;
		queue_infos.push_back(graphics_info);

		if (m_PresentFamily != m_GraphicsFamily)
		{
			VkDeviceQueueCreateInfo present_info = graphics_info;
			present_info.queueFamilyIndex = m_PresentFamily;
			queue_infos.push_back(present_info);
		}

		std::vector<const char*> device_extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

		VkPhysicalDeviceFeatures features{};

		VkDeviceCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_infos.size());
		create_info.pQueueCreateInfos = queue_infos.data();
		create_info.enabledExtensionCount = static_cast<uint32_t>(device_extensions.size());
		create_info.ppEnabledExtensionNames = device_extensions.data();
		create_info.pEnabledFeatures = &features;

		LM_VK_CHECK(vkCreateDevice(m_PhysicalDevice, &create_info, nullptr, &m_Device));
		if (m_Device == VK_NULL_HANDLE)
		{
			LM_CORE_ASSERT(false, "Failed to create the Vulkan logical device");
			return;
		}

		vkGetDeviceQueue(m_Device, m_GraphicsFamily, 0, &m_GraphicsQueue);
		vkGetDeviceQueue(m_Device, m_PresentFamily, 0, &m_PresentQueue);
	}

	uint32_t VKContext::FindMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties) const
	{
		for (uint32_t i = 0; i < m_MemoryProperties.memoryTypeCount; i++)
		{
			const bool type_allowed = (typeBits & (1u << i)) != 0;
			const bool has_properties = (m_MemoryProperties.memoryTypes[i].propertyFlags & properties) == properties;
			if (type_allowed && has_properties)
				return i;
		}

		LM_CORE_ASSERT(false, "No Vulkan memory type matches the requested properties");
		return 0;
	}

	void VKContext::Shutdown()
	{
		LM_PROFILE_FUNCTION();

		if (m_Device != VK_NULL_HANDLE)
		{
			vkDeviceWaitIdle(m_Device);
			vkDestroyDevice(m_Device, nullptr);
			m_Device = VK_NULL_HANDLE;
		}
		if (m_Surface != VK_NULL_HANDLE)
		{
			vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
			m_Surface = VK_NULL_HANDLE;
		}
		if (m_DebugMessenger != VK_NULL_HANDLE)
		{
			auto destroy =
			    reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(m_Instance, "vkDestroyDebugUtilsMessengerEXT"));
			if (destroy)
				destroy(m_Instance, m_DebugMessenger, nullptr);
			m_DebugMessenger = VK_NULL_HANDLE;
		}
		if (m_Instance != VK_NULL_HANDLE)
		{
			vkDestroyInstance(m_Instance, nullptr);
			m_Instance = VK_NULL_HANDLE;
		}

		m_PhysicalDevice = VK_NULL_HANDLE;
		m_GraphicsQueue = VK_NULL_HANDLE;
		m_PresentQueue = VK_NULL_HANDLE;
	}
}
