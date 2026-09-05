#include "LMPCH.h"
#include "VKPipelineCache.h"

#include "Lumora/Utilities/Hash.h"

namespace Lumora::Lumen
{
	VkFormat ToVkFormat(AttributeType type)
	{
		switch (type)
		{
		case AttributeType::Float: return VK_FORMAT_R32_SFLOAT;
		case AttributeType::Float2: return VK_FORMAT_R32G32_SFLOAT;
		case AttributeType::Float3: return VK_FORMAT_R32G32B32_SFLOAT;
		case AttributeType::Float4: return VK_FORMAT_R32G32B32A32_SFLOAT;
		case AttributeType::Int: return VK_FORMAT_R32_SINT;
		case AttributeType::Int2: return VK_FORMAT_R32G32_SINT;
		case AttributeType::Int3: return VK_FORMAT_R32G32B32_SINT;
		case AttributeType::Int4: return VK_FORMAT_R32G32B32A32_SINT;
		case AttributeType::UByte4Norm: return VK_FORMAT_R8G8B8A8_UNORM;
		}

		LM_CORE_ASSERT(false, "Unknown AttributeType")
		return VK_FORMAT_UNDEFINED;
	}

	uint64_t HashVertexLayout(const VertexLayout& layout)
	{
		// FNV-1a over the layout
		uint64_t hash = 14695981039346656037ull;
		const auto fold = [&hash](uint64_t value)
		{
			hash = (hash ^ value) * 1099511628211ull;
		};

		fold(layout.Stride);
		for (const VertexAttribute& attribute : layout.Attributes)
		{
			fold(static_cast<uint64_t>(attribute.Type));
			fold(attribute.Offset);
		}
		return hash;
	}

	void VKPipelineCache::Init(VKContext& context, VkRenderPass renderPass)
	{
		LM_PROFILE_FUNCTION();

		m_Context = &context;
		m_RenderPass = renderPass;

		std::array<VkDescriptorSetLayoutBinding, Bindings::MaxTextureSlots + Bindings::MaxUniformSlots> bindings{};

		for (uint32_t i = 0; i < Bindings::MaxTextureSlots; ++i)
		{
			bindings[i].binding = Bindings::Texture0 + i;
			bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			bindings[i].descriptorCount = 1;
			bindings[i].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT;
		}

		for (uint32_t i = 0; i < Bindings::MaxUniformSlots; ++i)
		{
			const uint32_t index = Bindings::MaxTextureSlots + i;
			bindings[index].binding = Bindings::Uniform0 + i;
			// DYNAMIC because the data lives in a per frame ring: the descriptor names the ring, the offset comers in at bind time.
			bindings[index].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
			bindings[index].descriptorCount = 1;
			bindings[index].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT;
		}

		VkDescriptorSetLayoutCreateInfo set_info{};
		set_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		set_info.bindingCount = static_cast<uint32_t>(bindings.size());
		set_info.pBindings = bindings.data();
		LM_VK_CHECK(vkCreateDescriptorSetLayout(m_Context->GetDevice(), &set_info, nullptr, &m_SetLayout));

		VkPipelineLayoutCreateInfo layout_info{};
		layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		layout_info.setLayoutCount = 1;
		layout_info.pSetLayouts = &m_SetLayout;
		LM_VK_CHECK(vkCreatePipelineLayout(m_Context->GetDevice(), &layout_info, nullptr, &m_PipelineLayout));
	}

	VkPipeline VKPipelineCache::GetOrCreate(uint32_t shaderId, VkShaderModule vertex, VkShaderModule fragment, const VertexLayout& layout)
	{
		LM_PROFILE_FUNCTION();

		const Key key = Key{shaderId, HashVertexLayout(layout)};

		auto it = m_Pipelines.find(key);
		if (it != m_Pipelines.end())
			return it->second;

		VkPipeline pipeline = Build(vertex, fragment, layout);
		if (pipeline == VK_NULL_HANDLE)
			return VK_NULL_HANDLE;

		m_Pipelines[key] = pipeline;
		LM_CORE_TRACE("Built Vulkan pipeline for shader {} ({} pipeline cached)", shaderId, m_Pipelines.size());
		return pipeline;
	}

	VkPipeline VKPipelineCache::GetOrCreateCompute(uint32_t shaderId, VkShaderModule compute)
	{
		LM_PROFILE_FUNCTION();

		auto it = m_ComputePipelines.find(shaderId);
		if (it != m_ComputePipelines.end())
			return it->second;

		VkComputePipelineCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		info.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		info.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		info.stage.module = compute;
		info.stage.pName = "main";
		info.layout = m_PipelineLayout;

		VkPipeline pipeline = VK_NULL_HANDLE;
		LM_VK_CHECK(vkCreateComputePipelines(m_Context->GetDevice(), VK_NULL_HANDLE, 1, &info, nullptr, &pipeline));
		if (pipeline == VK_NULL_HANDLE)
			return VK_NULL_HANDLE;

		m_ComputePipelines[shaderId] = pipeline;
		LM_CORE_TRACE("Built Vulkan compute pipeline for shader {}", shaderId);
		return pipeline;
	}

	VkPipeline VKPipelineCache::Build(VkShaderModule vertex, VkShaderModule fragment, const VertexLayout& layout) const
	{
		LM_PROFILE_FUNCTION();

		VkPipelineShaderStageCreateInfo stages[2] {};
		stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		stages[0].module = vertex;
		stages[0].pName = "main";
		stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		stages[1].module = fragment;
		stages[1].pName = "main";

		VkVertexInputBindingDescription binding{};
		binding.binding = 0;
		binding.stride = layout.Stride;
		binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		// Locations run 0..n in declaration order, matching glEnableVertexAttributeArray(i) on the GL side
		std::vector<VkVertexInputAttributeDescription> attributes(layout.Attributes.size());
		for (size_t i = 0; i < layout.Attributes.size(); i++)
		{
			attributes[i].location = static_cast<uint32_t>(i);
			attributes[i].binding = 0;
			attributes[i].format = ToVkFormat(layout.Attributes[i].Type);
			attributes[i].offset = layout.Attributes[i].Offset;
		}

		VkPipelineVertexInputStateCreateInfo vertex_input{};
		vertex_input.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertex_input.vertexBindingDescriptionCount = 1;
		vertex_input.pVertexBindingDescriptions = &binding;
		vertex_input.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributes.size());
		vertex_input.pVertexAttributeDescriptions = attributes.data();

		VkPipelineInputAssemblyStateCreateInfo input_assembly{};
		input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

		// Both are dynamic, so a resize never rebuilds a pipeline.
		VkPipelineViewportStateCreateInfo viewport_state{};
		viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewport_state.viewportCount = 1;
		viewport_state.scissorCount = 1;

		VkPipelineRasterizationStateCreateInfo rasterizer {};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		// GL_FACE_CULL is off in GLRenderDevice::Init, and it has to stay off here.
		rasterizer.cullMode = VK_CULL_MODE_NONE;
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizer.lineWidth = 1.0f;

		VkPipelineMultisampleStateCreateInfo multisample{};
		multisample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineDepthStencilStateCreateInfo depth_stencil{};
		depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depth_stencil.depthTestEnable = VK_TRUE;
		depth_stencil.depthWriteEnable = VK_TRUE;
		depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS;
		depth_stencil.maxDepthBounds = 1.0f;

		// glBlendFunc sets one pair for both colour and alpha, so both pairs match it here.
		VkPipelineColorBlendAttachmentState blend_attachment{};
		blend_attachment.blendEnable = VK_TRUE;
		blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
		blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;
		blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
		                                  VK_COLOR_COMPONENT_A_BIT;

		VkPipelineColorBlendStateCreateInfo blend{};
		blend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		blend.attachmentCount = 1;
		blend.pAttachments = &blend_attachment;

		constexpr VkDynamicState dynamic_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

		VkPipelineDynamicStateCreateInfo dynamic{};
		dynamic.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamic.dynamicStateCount = 2;
		dynamic.pDynamicStates = dynamic_states;


		VkGraphicsPipelineCreateInfo pipeline_info{};
		pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipeline_info.stageCount = 2;
		pipeline_info.pStages = stages;
		pipeline_info.pVertexInputState = &vertex_input;
		pipeline_info.pInputAssemblyState = &input_assembly;
		pipeline_info.pViewportState = &viewport_state;
		pipeline_info.pRasterizationState = &rasterizer;
		pipeline_info.pMultisampleState = &multisample;
		pipeline_info.pDepthStencilState = &depth_stencil;
		pipeline_info.pColorBlendState = &blend;
		pipeline_info.pDynamicState = &dynamic;
		pipeline_info.layout = m_PipelineLayout;
		pipeline_info.renderPass = m_RenderPass;
		pipeline_info.subpass = 0;

		VkPipeline pipeline = VK_NULL_HANDLE;
		LM_VK_CHECK(vkCreateGraphicsPipelines(m_Context->GetDevice(), VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &pipeline));
		return pipeline;
	}

	void VKPipelineCache::DestroyShaderPipeline(uint32_t shaderId)
	{ 
		LM_PROFILE_FUNCTION();

		for (auto it = m_Pipelines.begin(); it != m_Pipelines.end();)
		{
			if (it->first.Shader != shaderId)
			{
				++it;
				continue;
			}

			vkDestroyPipeline(m_Context->GetDevice(), it->second, nullptr);
			it = m_Pipelines.erase(it);
		}

		if (auto compute = m_ComputePipelines.find(shaderId); compute != m_ComputePipelines.end())
		{
			vkDestroyPipeline(m_Context->GetDevice(), compute->second, nullptr);
			m_ComputePipelines.erase(compute);
		}
	}

	void VKPipelineCache::Shutdown()
	{
		LM_PROFILE_FUNCTION();

		if (m_Context == nullptr)
			return;

		for (auto& pipeline : m_Pipelines | std::views::values)
			vkDestroyPipeline(m_Context->GetDevice(), pipeline, nullptr);
		m_Pipelines.clear();

		for (auto& pipeline : m_ComputePipelines | std::views::values)
			vkDestroyPipeline(m_Context->GetDevice(), pipeline, nullptr);
		m_ComputePipelines.clear();

		if (m_PipelineLayout != VK_NULL_HANDLE)
			vkDestroyPipelineLayout(m_Context->GetDevice(), m_PipelineLayout, nullptr);
		if (m_SetLayout != VK_NULL_HANDLE)
			vkDestroyDescriptorSetLayout(m_Context->GetDevice(), m_SetLayout, nullptr);

		m_PipelineLayout = VK_NULL_HANDLE;
		m_SetLayout = VK_NULL_HANDLE;
		m_RenderPass = VK_NULL_HANDLE;
		m_Context = nullptr;
	}
}
