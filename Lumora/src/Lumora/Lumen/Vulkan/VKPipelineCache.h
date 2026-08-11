#pragma once

#include "Lumora/Lumen/RenderTypes.h"
#include "Lumora/Lumen/Vulkan/VKContext.h"

namespace Lumora::Lumen
{
	class VKPipelineCache
	{
	public:
		void Init(VKContext& context, VkRenderPass renderPass);
		void Shutdown();

		VkDescriptorSetLayout GetSetLayout() const { return m_SetLayout; }
		VkPipelineLayout GetPipelineLayout() const { return m_PipelineLayout; }
		
		VkPipeline GetOrCreate(uint32_t shaderId, VkShaderModule vertex, VkShaderModule fragment, const VertexLayout& layout);
		void DestroyShaderPipeline(uint32_t shaderId);

	private:
		struct Key
		{
			uint32_t Shader = 0;
			uint64_t LayoutHash = 0;

			bool operator==(const Key& other) const = default;
		};

		struct KeyHash
		{
			size_t operator()(const Key& key) const
			{
				return std::hash<uint64_t>{}(key.LayoutHash ^ (static_cast<uint64_t>(key.Shader) << 32));
			}
		};

		VkPipeline Build(VkShaderModule vertex, VkShaderModule fragment, const VertexLayout& layout) const;

		VKContext* m_Context = nullptr;
		VkRenderPass m_RenderPass = VK_NULL_HANDLE;
		VkDescriptorSetLayout m_SetLayout = VK_NULL_HANDLE;
		VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;

		std::unordered_map<Key, VkPipeline, KeyHash> m_Pipelines;
	};

	VkFormat ToVkFormat(AttributeType type);
	uint64_t HashVertexLayout(const VertexLayout& layout);
}
