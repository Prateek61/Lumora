#include "LMPCH.h"
#include "VKRingAllocator.h"

namespace Lumora::Lumen
{
	void VKRingAllocator::Init(VKContext& context, VkDeviceSize blockSize, VkBufferUsageFlags usage)
	{
		LM_PROFILE_FUNCTION();

		m_Context = &context;
		m_BlockSize = blockSize;
		m_Usage = usage;

		AddBlock(blockSize);
	}

	bool VKRingAllocator::AddBlock(VkDeviceSize size)
	{
		LM_PROFILE_FUNCTION();

		Block block;
		block.Size = size;

		VkBufferCreateInfo buffer_info{};
		buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer_info.size = size;
		buffer_info.usage = m_Usage;
		buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		LM_VK_CHECK(vkCreateBuffer(m_Context->GetDevice(), &buffer_info, nullptr, &block.Buffer));
		if (block.Buffer == VK_NULL_HANDLE)
			return false;

		VkMemoryRequirements requirements{};
		vkGetBufferMemoryRequirements(m_Context->GetDevice(), block.Buffer, &requirements);

		VkMemoryAllocateInfo allocate_info{};
		allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocate_info.allocationSize = requirements.size;
		allocate_info.memoryTypeIndex = m_Context->FindMemoryType(requirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		                                                                                           VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		LM_VK_CHECK(vkAllocateMemory(m_Context->GetDevice(), &allocate_info, nullptr, &block.Memory));
		LM_VK_CHECK(vkBindBufferMemory(m_Context->GetDevice(), block.Buffer, block.Memory, 0));

		// Mapped for the block's whole life. Coherent memory needs no flush, so a write is a memcpy.
		LM_VK_CHECK(vkMapMemory(m_Context->GetDevice(), block.Memory, 0, size, 0, &block.Mapped));
		if (block.Mapped == nullptr)
			return false;

		m_Blocks.push_back(block);
		return true;
	}

	VKRingAllocator::Allocation VKRingAllocator::Allocate(VkDeviceSize size, VkDeviceSize alignment)
	{
		LM_PROFILE_FUNCTION();

		if (size == 0)
			return {};

		for (; m_CurrentBlock < m_Blocks.size(); m_CurrentBlock++)
		{
			Block& block = m_Blocks[m_CurrentBlock];
			const VkDeviceSize offset = AlignUp(block.Used, alignment);
			if (offset + size > block.Size)
				continue;

			block.Used = offset + size;
			return Allocation{block.Buffer, offset, static_cast<uint8_t*>(block.Mapped) + offset};
		}

		if (!AddBlock(std::max(m_BlockSize, size)))
		{
			LM_CORE_ERROR("Vulkan ring allocator could not grow, dropping a {} byte update", size);
			return {};
		}

		LM_CORE_TRACE("Vulkan ring allocator grew to {} blocks, {} KB", m_Blocks.size(), GetCapacity() / 1024);

		m_CurrentBlock = m_Blocks.size() - 1;
		Block& block = m_Blocks[m_CurrentBlock];
		block.Used = size; // A block starts at offset 0, which satisfies any alignment.
		return Allocation{block.Buffer, 0, block.Mapped};
	}

	void VKRingAllocator::Reset()
	{
		for (Block& block : m_Blocks)
			block.Used = 0;
		m_CurrentBlock = 0;
	}

	VkDeviceSize VKRingAllocator::GetCapacity() const
	{
		VkDeviceSize total = 0;
		for (const Block& block : m_Blocks)
			total += block.Size;
		return total;
	}

	void VKRingAllocator::Shutdown()
	{
		LM_PROFILE_FUNCTION();

		if (m_Context == nullptr)
			return;

		for (Block& block : m_Blocks)
		{
			if (block.Mapped != nullptr)
				vkUnmapMemory(m_Context->GetDevice(), block.Memory);
			if (block.Buffer != VK_NULL_HANDLE)
				vkDestroyBuffer(m_Context->GetDevice(), block.Buffer, nullptr);
			if (block.Memory != VK_NULL_HANDLE)
				vkFreeMemory(m_Context->GetDevice(), block.Memory, nullptr);
		}

		m_Blocks.clear();
		m_CurrentBlock = 0;
		m_Context = nullptr;
	}
}
