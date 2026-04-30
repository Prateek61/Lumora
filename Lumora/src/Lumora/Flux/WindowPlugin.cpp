#include "LMPCH.h"
#include "WindowPlugin.h"

#include "Lumora/Core/Application.h"
#include "Lumora/Flux/Window.h"
#include "Lumora/Flux/Input.h"
#include "Lumora/Flux/Events.h"
#include "Lumora/Core/Events.h"

#include <GLFW/glfw3.h>

namespace
{
	using namespace Lumora;
	using namespace Lumora::Flux;

	void ProcessEvent(const Raw::KeyAction& event, KeyboardState& keyboardState, MouseState&,
	                  Aether::World&)
	{
		if (event.Key >= Key::MaxKeyCode)
			return;

		switch (event.Action)
		{
		case GLFW_PRESS: keyboardState.Keys[event.Key] = true;
			keyboardState.JustPressed[event.Key] = true;
			break;
		case GLFW_RELEASE: keyboardState.Keys[event.Key] = false;
			keyboardState.JustReleased[event.Key] = true;
			break;
		default: break;
		}
	}

	void ProcessEvent(const Raw::MouseButton& event, KeyboardState&, MouseState& mouseState,
	                  Aether::World&)
	{
		if (event.Button >= Mouse::MaxButtonCode)
			return;
		switch (event.Action)
		{
		case GLFW_PRESS: mouseState.Buttons[event.Button] = true;
			mouseState.JustPressed[event.Button] = true;
			break;
		case GLFW_RELEASE: mouseState.Buttons[event.Button] = false;
			mouseState.JustReleased[event.Button] = true;
			break;
		default: break;
		}
	}

	void ProcessEvent(const Raw::MouseMove& event, KeyboardState&, MouseState& mouseState, Aether::World&)
	{
		mouseState.DeltaX += event.X - mouseState.X;
		mouseState.DeltaY += event.Y - mouseState.Y;
		mouseState.X = event.X;
		mouseState.Y = event.Y;
	}

	void ProcessEvent(const Raw::MouseScroll& event, KeyboardState&, MouseState& mouseState, Aether::World&)
	{
		mouseState.ScrollX += event.XOffset;
		mouseState.ScrollY += event.YOffset;
	}

	void ProcessEvent(const Raw::WindowResize& event, KeyboardState&, MouseState&, Aether::World& world)
	{
		auto& windowRes = world.GetResourceMut<WindowResource>();
		windowRes.Resource->UpdateSize(event.Width, event.Height);
		world.GetResourceMut<Core::Events<WindowResize>>().Send({event.Width, event.Height});
	}

	void ProcessEvent(const Raw::WindowClose&, KeyboardState&, MouseState&,
	                  Aether::World& world)
	{
		LM_CORE_INFO("Window Close Event Received. Quitting Application.");
		world.GetResourceMut<Core::Events<WindowClose>>().Send({});
		world.Quit();
	}

	void ProcessEvent(const Raw::WindowFocus& event, KeyboardState&, MouseState&,
	                  Aether::World& world)
	{
		world.GetResourceMut<Core::Events<WindowFocus>>().Send(WindowFocus{.Focused = event.Focused});
	}

	void ProcessEvent(const Raw::CharTyped&, KeyboardState&, MouseState&, Aether::World&)
	{
		// For now, we ignore CharTyped events. They can be used for text input later on.
	}
}

namespace Lumora::Flux
{
	WindowPlugin::WindowPlugin(WindowProps props)
		: m_InitialProps(std::move(props)) {}

	void WindowPlugin::Build(Core::Application& app)
	{
		LM_PROFILE_FUNCTION();

		Aether::World& world = app.GetWorld();

		// Create the window and insert it as a resource
		world.SetResource(WindowResource{CreateScope<Window>(m_InitialProps)});
		world.SetResource(KeyboardState{});
		world.SetResource(MouseState{});

		// Define the structure of the resources
		world.Raw()
		     .component<MouseState>()
		     .member("X", &MouseState::X)
		     .member("Y", &MouseState::Y)
		     .member("DeltaX", &MouseState::DeltaX)
		     .member("DeltaY", &MouseState::DeltaY)
		     .member("ScrollX", &MouseState::ScrollX)
		     .member("ScrollY", &MouseState::ScrollY);

		// Setup Events :Resize, Close, Focus etc.
		world.SetResource(Core::Events<WindowResize>{});
		world.SetResource(Core::Events<WindowClose>{});
		world.SetResource(Core::Events<WindowFocus>{});

		// Set up Callback
		auto callbackFn = [this](const Raw::RawEvent& event)
		{
			m_EventBuffer.push_back(event);
		};

		// Add the callback to the window
		auto& win = world.GetResource<WindowResource>().Resource;
		win->SetupCallback(callbackFn);

		// Add a system that polls events and processes the event buffer
		auto poll_system_builder = world.System("Flux::PollAndProcessEvents");
		poll_system_builder.SetPhase<Aether::Phases::Input>();
		poll_system_builder.Write<WindowResource>().Write<KeyboardState>().Write<MouseState>();
		m_WindowEventPollingSystem = poll_system_builder.Run([this](Aether::QueryRes& res)
		{
			auto world = res.World();

			// 1. Get resources
			auto& window_res = world.GetResourceMut<WindowResource>();
			auto& keyboard_state = world.GetResourceMut<KeyboardState>();
			auto& mouse_state = world.GetResourceMut<MouseState>();
			auto& window_resize_event = world.GetResourceMut<Core::Events<WindowResize>>();
			auto& window_close_event = world.GetResourceMut<Core::Events<WindowClose>>();
			auto& window_focus_event = world.GetResourceMut<Core::Events<WindowFocus>>();

			// 2. Reset per-frame input state and Event buffer
			keyboard_state.ResetFrame();
			mouse_state.ResetFrame();
			window_resize_event.Clear();
			window_close_event.Clear();
			window_focus_event.Clear();

			// 3. Poll GLFW events (this will trigger our callbacks and fill the event buffer)
			window_res.Resource->PollEvents();

			// 4. Drain the buffer and update state
			for (const auto& event : m_EventBuffer)
			{
				std::visit([&](const auto& e) { ProcessEvent(e, keyboard_state, mouse_state, world); }, event);
			}
			m_EventBuffer.clear();
		});
	}

	void WindowPlugin::Cleanup(Core::Application& app)
	{
		LM_PROFILE_FUNCTION();

		auto& world = app.GetWorld();
		world.GetResourceMut<WindowResource>().Resource.reset();
	}
}
