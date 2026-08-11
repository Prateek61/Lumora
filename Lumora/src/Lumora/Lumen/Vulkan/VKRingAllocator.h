#pragma once

#include "Lumora/Lumen/Vulkan/VKContext.h"

namespace Lumora::Lumen
{
	class VKRingAllocator
	{
	public:
		struct Allocation
		{
			VkBuffer Buffer = VK_NULL_HANDLE;
			VkDeviceSize Offset = 0;
			void* Mapped = nullptr;

			bool IsValid() const { return Buffer != VK_NULL_HANDLE; }
		};

		void Init(VKContext& context, VkDeviceSize blockSize, VkBufferUsageFlags usage);
		void Shutdown();

		void Reset();
		Allocation Allocate(VkDeviceSize size, VkDeviceSize alignment);

		VkDeviceSize GetCapacity() const;

	private:
		struct Block
		{
			VkBuffer Buffer = VK_NULL_HANDLE;
			VkDeviceMemory Memory = VK_NULL_HANDLE;
			void* Mapped = nullptr;
			VkDeviceSize Size = 0;
			VkDeviceSize Used = 0;
		};

		bool AddBlock(VkDeviceSize size);
		VKContext* m_Context = nullptr;
		VkDeviceSize m_BlockSize = 0;
		VkBufferUsageFlags m_Usage = 0;

		std::vector<Block> m_Blocks;
		size_t m_CurrentBlock = 0;
	};
}