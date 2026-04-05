#pragma once

#include "Lumora/Flux/KeyCodes.h"
#include "Lumora/Flux/MouseCodes.h"

#include <variant>
#include <vector>
#include <cstdint>

namespace Lumora::Flux
{
	namespace Raw
	{
		struct WindowResize
		{
			uint32_t Width, Height;
		};

		struct WindowClose
		{
		};

		struct WindowFocus
		{
			bool Focused;
		};

		struct KeyAction
		{
			KeyCode Key;
			int Action;
			int Mods;
		};

		struct CharTyped
		{
			uint32_t Codepoint;
		};

		struct MouseMove
		{
			float X, Y;
		};

		struct MouseButton
		{
			MouseCode Button;
			int Action;
			int Mods;
		};

		struct MouseScroll
		{
			float XOffset, YOffset;
		};

		using RawEvent = std::variant<
			WindowResize,
			WindowClose,
			WindowFocus,
			KeyAction,
			CharTyped,
			MouseMove,
			MouseButton,
			MouseScroll
		>;

		using RawEventBuffer = std::vector<RawEvent>;
	}
}
