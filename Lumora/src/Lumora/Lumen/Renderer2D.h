#pragma once

#include "Lumora/Lumen/RenderDevice.h"
#include "Lumora/Lumen/RenderTypes.h"
#include <GLM/glm.hpp>

namespace Lumora::Lumen
{
	class Renderer2D
	{
	public:
		static constexpr uint32_t MaxQuads = 10000;
		static constexpr uint32_t MaxVertices = MaxQuads * 4;
		static constexpr uint32_t MaxIndices = MaxQuads * 6;
		static constexpr uint32_t MaxTextureSlots = 16; // Typical GPU minimum

		// Call Once
		void Init(RenderDevice& device);
		void Shutdown();

		// Frame API
		void Begin(const glm::mat4& viewProjMatrix);
		void End();
		void Flush(); // Manually flush if you want to draw before End() or if you want to ensure stats are updated immediately

		// Draw Calls — Solid Color
		void DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);
		void DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color);

		// Draw Calls — Textured (with optional color tint, default white = no tint)
		void DrawTexturedQuad(const glm::vec2& position, const glm::vec2& size, TextureHandle texture, const glm::vec4& tint = glm::vec4(1.0f));
		void DrawTexturedQuad(const glm::vec3& position, const glm::vec2& size, TextureHandle texture, const glm::vec4& tint = glm::vec4(1.0f));

		// Stats
		struct Stats
		{
			uint32_t DrawCalls = 0;
			uint32_t QuadCount = 0;
		};
		Stats GetStats() const { return m_Stats; }
		void ResetStats() { m_Stats = {}; }
	private:
		RenderDevice* m_Device = nullptr;

		BufferHandle m_VertexBuffer;
		BufferHandle m_IndexBuffer;
		ShaderHandle m_Shader;
		TextureHandle m_WhiteTexture;
		BufferHandle m_UniformBuffer;

		// Texture slot management
		std::array<TextureHandle, MaxTextureSlots> m_TextureSlots{}; // Slot 0 = white texture
		uint32_t m_TextureSlotIndex = 1; // Next free slot (0 is always white)

		// CPU-side Quad Data
		std::vector<QuadVertex> m_QuadVertexData;
		uint32_t m_QuadCount = 0;

		Stats m_Stats;

		int FindOrAssignTextureSlot(TextureHandle texture);
	};

	namespace DefaultShaders
	{
		const char* GetQuadVertexShader();
		const char* GetQuadFragmentShader();
	}
}