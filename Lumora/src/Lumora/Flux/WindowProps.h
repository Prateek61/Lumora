#pragma once

#include <string>
#include <cstdint>

namespace Lumora::Flux
{
	struct WindowProps
	{
		std::string Title = "Lumora Window";
		uint32_t Width = 1280;
		uint32_t Height = 720;
		bool VSync = true;
	};
}