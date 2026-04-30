#pragma once

#include <cstdint>

namespace Lumora::Flux
{
	struct WindowResize
	{
		uint32_t Width, Height;
	};

	struct WindowFocus
	{
		bool Focused;
	};

	struct WindowClose
	{};
}