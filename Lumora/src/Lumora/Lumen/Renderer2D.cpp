#include "LMPCH.h"
#include "Renderer2D.h"

namespace Lumora::Lumen
{
	namespace DefaultShaders
	{
		const char* GetQuadVertexShader()
		{
			return R"(
				#version 450 core
				layout(location = 0) in vec3 a_Position;
				layout(location = 1) in vec2 a_TexCoord;
				layout(location = 2) in vec4 a_Color;
				layout(location = 3) in int a_TexIndex;

				layout(std140, binding = 0) uniform CameraBuffer
				{
					mat4 u_ViewProj;
				};

				out vec2 v_TexCoord;
				out vec4 v_Color;
				flat out int v_TexIndex;

				void main()
				{
					v_TexCoord = a_TexCoord;
					v_Color = a_Color;
					v_TexIndex = a_TexIndex;
					gl_Position = u_ViewProj * vec4(a_Position, 1.0);
				}
			)";
		}

		const char* GetQuadFragmentShader()
		{
			return R"(
				#version 450 core

				in vec2 v_TexCoord;
				in vec4 v_Color;
				flat in int v_TexIndex;

				layout(location = 0) out vec4 FragColor;

				layout(binding = 0) uniform sampler2D u_Tex0;
				layout(binding = 1) uniform sampler2D u_Tex1;
				layout(binding = 2) uniform sampler2D u_Tex2;
				layout(binding = 3) uniform sampler2D u_Tex3;
				layout(binding = 4) uniform sampler2D u_Tex4;
				layout(binding = 5) uniform sampler2D u_Tex5;
				layout(binding = 6) uniform sampler2D u_Tex6;
				layout(binding = 7) uniform sampler2D u_Tex7;
				layout(binding = 8) uniform sampler2D u_Tex8;
				layout(binding = 9) uniform sampler2D u_Tex9;
				layout(binding = 10) uniform sampler2D u_Tex10;
				layout(binding = 11) uniform sampler2D u_Tex11;
				layout(binding = 12) uniform sampler2D u_Tex12;
				layout(binding = 13) uniform sampler2D u_Tex13;
				layout(binding = 14) uniform sampler2D u_Tex14;
				layout(binding = 15) uniform sampler2D u_Tex15;

				vec4 sampleTexture(int index, vec2 uv)
				{
					switch (index)
					{
						case 0:  return texture(u_Tex0, uv);
						case 1:  return texture(u_Tex1, uv);
						case 2:  return texture(u_Tex2, uv);
						case 3:  return texture(u_Tex3, uv);
						case 4:  return texture(u_Tex4, uv);
						case 5:  return texture(u_Tex5, uv);
						case 6:  return texture(u_Tex6, uv);
						case 7:  return texture(u_Tex7, uv);
						case 8:  return texture(u_Tex8, uv);
						case 9:  return texture(u_Tex9, uv);
						case 10: return texture(u_Tex10, uv);
						case 11: return texture(u_Tex11, uv);
						case 12: return texture(u_Tex12, uv);
						case 13: return texture(u_Tex13, uv);
						case 14: return texture(u_Tex14, uv);
						case 15: return texture(u_Tex15, uv);
						default: return texture(u_Tex0, uv);
					}
				}

				void main()
				{
					FragColor = sampleTexture(v_TexIndex, v_TexCoord) * v_Color;
				}
			)";
		}
	}

	void Renderer2D::Init(RenderDevice& device)
	{
		LM_PROFILE_FUNCTION();

		m_Device = &device;

		// Create dynamic vertex buffer (no data yet - filled each frame)
		auto layout = QuadVertex::GetLayout();
		m_VertexBuffer = device.CreateVertexBuffer(nullptr, MaxVertices * layout.Stride, layout, true);

		// Create index buffer - static, quad indices never change
		// Every quad is made of 2 triangles (6 indices)
		// 0, 1, 2, 2, 3, 0
		std::vector<uint32_t> indices(MaxIndices);
		for (uint32_t i = 0; i < MaxQuads; i++)
		{
			uint32_t offset = i * 4;
			indices[i * 6 + 0] = offset + 0;
			indices[i * 6 + 1] = offset + 1;
			indices[i * 6 + 2] = offset + 2;
			indices[i * 6 + 3] = offset + 2;
			indices[i * 6 + 4] = offset + 3;
			indices[i * 6 + 5] = offset + 0;
		}
		m_IndexBuffer = device.CreateIndexBuffer(indices.data(), MaxIndices);

		// Compile the default quad shader
		m_Shader = device.CreateShader(DefaultShaders::GetQuadVertexShader(), DefaultShaders::GetQuadFragmentShader());
		LM_CORE_ASSERT(m_Shader.IsValid(), "Failed to create default quad shader!")

		// Create a 1x1 white texture for untextured quads
		uint32_t whitePixel = 0xFFFFFFFF; // RGBA
		m_WhiteTexture = device.CreateTexture2D(1, 1, &whitePixel, 4);

		// Slot 0 is always the white texture
		m_TextureSlots[0] = m_WhiteTexture;
		m_TextureSlotIndex = 1;

		// Create uniform buffer for camera data
		m_UniformBuffer = device.CreateUniformBuffer(sizeof(glm::mat4));

		LM_CORE_INFO("Renderer2D initialized with max {} quads ({} vertices, {} indices)", MaxQuads, MaxVertices, MaxIndices);
	}

	void Renderer2D::Shutdown()
	{
		LM_PROFILE_FUNCTION();

		m_Device->DestroyBuffer(m_VertexBuffer);
		m_Device->DestroyBuffer(m_IndexBuffer);
		m_Device->DestroyShader(m_Shader);
		m_Device->DestroyTexture(m_WhiteTexture);
		m_Device->DestroyBuffer(m_UniformBuffer);
	}

	void Renderer2D::Begin(const glm::mat4& viewProjMatrix)
	{
		LM_PROFILE_FUNCTION();

		m_Device->BindUniformBuffer(m_UniformBuffer, 0);
		m_Device->UpdateUniformBuffer(m_UniformBuffer, &viewProjMatrix, sizeof(glm::mat4), 0);

		m_QuadVertexData.clear();
		m_QuadCount = 0;
		m_TextureSlotIndex = 1; // Reset texture slots (keep slot 0 = white)
		ResetStats();
	}

	void Renderer2D::End()
	{
		LM_PROFILE_FUNCTION();

		if (m_QuadCount == 0)
			return; // Nothing to draw
		Flush();
	}

	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
	{
		DrawQuad(glm::vec3(position, 0.0f), size, color);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color)
	{
		LM_PROFILE_FUNCTION();

		if (m_QuadCount >= MaxQuads)
			Flush();

		constexpr int texIndex = 0; // White texture

		float x2 = position.x + size.x;
		float y2 = position.y + size.y;

		m_QuadVertexData.emplace_back(QuadVertex{ {position.x, position.y, position.z}, {0.0f, 0.0f}, color, texIndex });
		m_QuadVertexData.emplace_back(QuadVertex{ {x2, position.y, position.z},          {1.0f, 0.0f}, color, texIndex });
		m_QuadVertexData.emplace_back(QuadVertex{ {x2, y2, position.z},                  {1.0f, 1.0f}, color, texIndex });
		m_QuadVertexData.emplace_back(QuadVertex{ {position.x, y2, position.z},          {0.0f, 1.0f}, color, texIndex });

		m_QuadCount++;
	}

	void Renderer2D::DrawTexturedQuad(const glm::vec2& position, const glm::vec2& size, TextureHandle texture, const glm::vec4& tint)
	{
		DrawTexturedQuad(glm::vec3(position, 0.0f), size, texture, tint);
	}

	void Renderer2D::DrawTexturedQuad(const glm::vec3& position, const glm::vec2& size, TextureHandle texture, const glm::vec4& tint)
	{
		LM_PROFILE_FUNCTION();

		if (m_QuadCount >= MaxQuads)
			Flush();

		int texIndex = FindOrAssignTextureSlot(texture);

		float x2 = position.x + size.x;
		float y2 = position.y + size.y;

		m_QuadVertexData.emplace_back(QuadVertex{ {position.x, position.y, position.z}, {0.0f, 0.0f}, tint, texIndex });
		m_QuadVertexData.emplace_back(QuadVertex{ {x2, position.y, position.z},          {1.0f, 0.0f}, tint, texIndex });
		m_QuadVertexData.emplace_back(QuadVertex{ {x2, y2, position.z},                  {1.0f, 1.0f}, tint, texIndex });
		m_QuadVertexData.emplace_back(QuadVertex{ {position.x, y2, position.z},          {0.0f, 1.0f}, tint, texIndex });

		m_QuadCount++;
	}

	int Renderer2D::FindOrAssignTextureSlot(TextureHandle texture)
	{
		// Check if texture is already bound in a slot
		for (uint32_t i = 1; i < m_TextureSlotIndex; i++)
		{
			if (m_TextureSlots[i].Id == texture.Id)
				return static_cast<int>(i);
		}

		// All slots full - flush current batch and reset
		if (m_TextureSlotIndex >= MaxTextureSlots)
			Flush();

		// Assign to next free slot
		int slot = static_cast<int>(m_TextureSlotIndex);
		m_TextureSlots[m_TextureSlotIndex] = texture;
		m_TextureSlotIndex++;
		return slot;
	}

	void Renderer2D::Flush()
	{
		if (m_QuadCount == 0)
			return;

		uint32_t vertex_data_size = static_cast<uint32_t>(m_QuadVertexData.size()) * sizeof(QuadVertex);
		m_Device->UpdateVertexBuffer(m_VertexBuffer, m_QuadVertexData.data(), vertex_data_size, 0);
		uint32_t index_count = m_QuadCount * 6;

		m_Device->BindShader(m_Shader);
		m_Device->BindUniformBuffer(m_UniformBuffer, 0);

		// Bind all active texture slots
		for (uint32_t i = 0; i < m_TextureSlotIndex; i++)
			m_Device->BindTexture(m_TextureSlots[i], i);

		m_Device->DrawIndexed(m_VertexBuffer, m_IndexBuffer, index_count);

		m_Stats.DrawCalls++;
		m_Stats.QuadCount += m_QuadCount;

		m_QuadVertexData.clear();
		m_QuadCount = 0;
		m_TextureSlotIndex = 1; // Reset slots (keep 0 = white)
	}
}