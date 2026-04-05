#pragma once

#include "Lumora/Flux/KeyCodes.h"
#include "Lumora/Flux/MouseCodes.h"

#include <array>

// Forward Declaration
namespace Lumora::Aether
{
	class World;
}

namespace Lumora::Flux
{
	struct KeyboardState
	{
		std::array<bool, Key::MaxKeyCode> Keys{};
		std::array<bool, Key::MaxKeyCode> JustPressed{};
		std::array<bool, Key::MaxKeyCode> JustReleased{};

		bool Down(KeyCode key) const { return key < Key::MaxKeyCode && Keys[key]; }
		bool Pressed(KeyCode key) const { return key < Key::MaxKeyCode && JustPressed[key]; }
		bool Released(KeyCode key) const { return key < Key::MaxKeyCode && JustReleased[key]; }

		void ResetFrame()
		{
			JustPressed.fill(false);
			JustReleased.fill(false);
		}
	};

	struct MouseState
	{
		float X = 0.0f;
		float Y = 0.0f;

		// Movement since last frame (accumulated from all MouseMove events)
		float DeltaX = 0.0f;
		float DeltaY = 0.0f;

		// Scroll this frame (accumulated - multiple scroll events per frame are possible)
		float ScrollX = 0.0f;
		float ScrollY = 0.0f;

		// Button states
		std::array<bool, Mouse::MaxButtonCode> Buttons{};
		std::array<bool, Mouse::MaxButtonCode> JustPressed{};
		std::array<bool, Mouse::MaxButtonCode> JustReleased{};

		bool Down(MouseCode button) const { return button < Mouse::MaxButtonCode && Buttons[button]; }
		bool Pressed(MouseCode button) const { return button < Mouse::MaxButtonCode && JustPressed[button]; }
		bool Released(MouseCode button) const { return button < Mouse::MaxButtonCode && JustReleased[button]; }

		void ResetFrame()
		{
			DeltaX = 0.0f;
			DeltaY = 0.0f;
			ScrollX = 0.0f;
			ScrollY = 0.0f;
			JustPressed.fill(false);
			JustReleased.fill(false);
		}
	};

	class Input
	{
	public:
		const KeyboardState& Keyboard;
		const MouseState& Mouse;

	public:
		Input(const KeyboardState& keyboardState, const MouseState& mouseState)
			: Keyboard(keyboardState), Mouse(mouseState)
		{
		}

		static Input Get(Aether::World& world);
	};
}