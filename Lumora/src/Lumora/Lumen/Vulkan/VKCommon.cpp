#include "LMPCH.h"
#include "VKCommon.h"

#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan_core.h>

namespace Lumora::Lumen
{
	const char* VkResultToString(VkResult result)
	{
		return string_VkResult(result);
	}
}