#pragma once

#include <string>
#include <cstdint>
#include "Lumora/Lumen/RenderAPI.h"
#include "Lumora/Rune/Serialization/Reflect.h"

namespace Lumora::Flux
{
	struct WindowProps
	{
		std::string Title = "Lumora Window";
		uint32_t Width = 1280;
		uint32_t Height = 720;
		Lumen::RenderAPI API = Lumen::RenderAPI::OpenGL;
		bool VSync = true;
	};
}
VISITABLE_STRUCT(Lumora::Flux::WindowProps, Title, Width, Height, VSync);