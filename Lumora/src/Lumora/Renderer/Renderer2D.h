#pragma once

#include "Lumora/Renderer/Renderer.h"
#include "glm/glm.hpp"

namespace Lumora
{
	class Renderer2D
	{
	public:
		static void Init();
		static void Shutdown(){}

		static void BeginFrame(){}
		static void EndFrame(){}

		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, uint32_t rgba){}
	};
}