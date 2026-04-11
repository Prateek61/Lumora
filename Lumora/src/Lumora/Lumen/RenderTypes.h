#pragma once

#include <cstdint>
#include <vector>
#include "glm/glm.hpp"

namespace Lumora::Lumen
{
	struct BufferHandle
	{
		uint32_t Id = 0;
		bool IsValid() const { return Id != 0; }
	};

	struct ShaderHandle
	{
		uint32_t Id = 0;
		bool IsValid() const { return Id != 0; }
	};

	struct TextureHandle
	{
		uint32_t Id = 0;
		bool IsValid() const { return Id != 0; }
	};

	enum class AttributeType : uint8_t
	{
		Float, Float2, Float3, Float4,
		Int, Int2, Int3, Int4,
		UByte4Norm // Unsigned Byte 4 Normalized (e.g., for colors)
	};

	struct VertexAttribute
	{
		AttributeType Type;
		uint32_t Offset; // Byte offset in the vertex structure

		static uint32_t SizeOf(AttributeType type)
		{
			switch (type)
			{
			case AttributeType::Float: return 4;
			case AttributeType::Float2: return 8;
			case AttributeType::Float3: return 12;
			case AttributeType::Float4: return 16;
			case AttributeType::Int: return 4;
			case AttributeType::Int2: return 8;
			case AttributeType::Int3: return 12;
			case AttributeType::Int4: return 16;
			case AttributeType::UByte4Norm: return 4;
			}
			LM_CORE_ASSERT(false, "Unknown AttributeType");
			return 0;
		}
	};

	struct VertexLayout
	{
		uint32_t Stride; // Total size of one vertex in bytes
		std::vector<VertexAttribute> Attributes;

		VertexLayout& Add(AttributeType type)
		{
			Attributes.emplace_back(type, Stride);
			Stride += VertexAttribute::SizeOf(type);
			return *this;
		}
	};

	struct QuadVertex
	{
		glm::vec3 Position;
		glm::vec2 TexCoord;
		glm::vec4 Color;
		int TexIndex = 0; // Index into bound texture slots (0 = white/solid color)

		static VertexLayout GetLayout()
		{
			return VertexLayout()
				.Add(AttributeType::Float3) // Position
				.Add(AttributeType::Float2) // TexCoord
				.Add(AttributeType::Float4) // Color
				.Add(AttributeType::Int);  // TexIndex
		}
	};
}
