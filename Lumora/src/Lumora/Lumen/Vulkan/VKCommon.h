#pragma once

#include <vulkan/vulkan_core.h>
#include "Lumora/Core/Log.h"

namespace Lumora::Lumen
{
	const char* VkResultToString(VkResult result);

	constexpr VkDeviceSize AlignUp(VkDeviceSize value, VkDeviceSize alignment)
	{
		return alignment == 0 ? value : (value + alignment - 1) & ~(alignment - 1);
	}
}

#define LM_VK_CHECK(expr)                                                              \
	do                                                                                 \
	{                                                                                  \
		VkResult lm_vk_result = (expr);                                                \
		if (lm_vk_result != VK_SUCCESS)                                                \
			LM_CORE_ERROR("Vulkan call failed: {} -> {}", #expr,                       \
							::Lumora::Lumen::VkResultToString(lm_vk_result));          \
	} while (0)
