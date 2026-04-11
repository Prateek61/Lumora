#pragma once

#include "Lumora/Lumen/RenderAPI.h"

namespace Lumora::Lumen
{
	struct RendererProps
	{
		RenderAPI API = RenderAPI::OpenGL;
	};

	struct Renderer2DProps
	{
	};
}