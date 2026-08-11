#pragma once

#include "Lumora/Lumen/Vulkan/VKCommon.h"

struct GLFWwindow;

namespace Lumora::Lumen
{
	class VKContext
	{
	public:
		void Init(GLFWwindow* window);
		void Shutdown();

		VkInstance GetInstance() const { return m_Instance; }
		VkSurfaceKHR GetSurface() const { return m_Surface; }
		VkPhysicalDevice GetPhysicalDevice() const { return m_PhysicalDevice; }
		VkDevice GetDevice() const { return m_Device; }
		VkQueue GetGraphicsQueue() const { return m_GraphicsQueue; }
		VkQueue GetPresentQueue() const { return m_PresentQueue; }
		uint32_t GetGraphicsFamily() const { return m_GraphicsFamily; }
		uint32_t GetPresentFamily() const { return m_PresentFamily; }
		const VkPhysicalDeviceProperties& GetDeviceProperties() const { return m_DeviceProperties; }

		bool IsValid() const { return m_Device != VK_NULL_HANDLE; }

		uint32_t FindMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties) const;

	private:
		bool CreateInstance();
		void CreateDebugMessenger();
		bool PickPhysicalDevice();
		void CreateLogicalDevice();

		VkInstance m_Instance = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;
		bool m_ValidationEnabled = false;
		VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
		VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
		VkDevice m_Device = VK_NULL_HANDLE;
		VkQueue m_GraphicsQueue = VK_NULL_HANDLE;
		VkQueue m_PresentQueue = VK_NULL_HANDLE;

		uint32_t m_GraphicsFamily = UINT32_MAX;
		uint32_t m_PresentFamily = UINT32_MAX;
		
		VkPhysicalDeviceProperties m_DeviceProperties {};
		VkPhysicalDeviceMemoryProperties m_MemoryProperties {};
	};
}