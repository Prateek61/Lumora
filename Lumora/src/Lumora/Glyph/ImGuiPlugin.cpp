#include "LMPCH.h"
#include "ImGuiPlugin.h"

#include "Lumora/Aether/System.h"
#include "Lumora/Flux/Window.h"
#include "Lumora/Flux/WindowPlugin.h"
#include "Lumora/Flux/Input.h"
#include "Lumora/Core/Application.h"
#include "Lumora/Lumen/RendererPlugin.h"
#include "Lumora/Glyph/ImGuiBackend.h"

#include <imgui.h>

namespace
{
	using namespace Lumora::Flux;
	using namespace Lumora;

	[[nodiscard]] ImGuiKey KeyCodeToImGuiKey(KeyCode key) noexcept
	{
		switch (key)
		{
		// Navigation
		case Key::Tab: return ImGuiKey_Tab;
		case Key::Left: return ImGuiKey_LeftArrow;
		case Key::Right: return ImGuiKey_RightArrow;
		case Key::Up: return ImGuiKey_UpArrow;
		case Key::Down: return ImGuiKey_DownArrow;
		case Key::PageUp: return ImGuiKey_PageUp;
		case Key::PageDown: return ImGuiKey_PageDown;
		case Key::Home: return ImGuiKey_Home;
		case Key::End: return ImGuiKey_End;
		case Key::Insert: return ImGuiKey_Insert;
		case Key::Delete: return ImGuiKey_Delete;
		case Key::Backspace: return ImGuiKey_Backspace;
		case Key::Enter: return ImGuiKey_Enter;
		case Key::KPEnter: return ImGuiKey_KeypadEnter;
		case Key::Escape: return ImGuiKey_Escape;

		// Whitespace / punctuation
		case Key::Space: return ImGuiKey_Space;
		case Key::Apostrophe: return ImGuiKey_Apostrophe;
		case Key::Comma: return ImGuiKey_Comma;
		case Key::Minus: return ImGuiKey_Minus;
		case Key::Period: return ImGuiKey_Period;
		case Key::Slash: return ImGuiKey_Slash;
		case Key::Semicolon: return ImGuiKey_Semicolon;
		case Key::Equal: return ImGuiKey_Equal;
		case Key::LeftBracket: return ImGuiKey_LeftBracket;
		case Key::Backslash: return ImGuiKey_Backslash;
		case Key::RightBracket: return ImGuiKey_RightBracket;
		case Key::GraveAccent: return ImGuiKey_GraveAccent;

		// Lock / utility
		case Key::CapsLock: return ImGuiKey_CapsLock;
		case Key::ScrollLock: return ImGuiKey_ScrollLock;
		case Key::NumLock: return ImGuiKey_NumLock;
		case Key::PrintScreen: return ImGuiKey_PrintScreen;
		case Key::Pause: return ImGuiKey_Pause;
		case Key::Menu: return ImGuiKey_Menu;

		// Digits
		case Key::D0: return ImGuiKey_0;
		case Key::D1: return ImGuiKey_1;
		case Key::D2: return ImGuiKey_2;
		case Key::D3: return ImGuiKey_3;
		case Key::D4: return ImGuiKey_4;
		case Key::D5: return ImGuiKey_5;
		case Key::D6: return ImGuiKey_6;
		case Key::D7: return ImGuiKey_7;
		case Key::D8: return ImGuiKey_8;
		case Key::D9: return ImGuiKey_9;

		// Letters
		case Key::A: return ImGuiKey_A;
		case Key::B: return ImGuiKey_B;
		case Key::C: return ImGuiKey_C;
		case Key::D: return ImGuiKey_D;
		case Key::E: return ImGuiKey_E;
		case Key::F: return ImGuiKey_F;
		case Key::G: return ImGuiKey_G;
		case Key::H: return ImGuiKey_H;
		case Key::I: return ImGuiKey_I;
		case Key::J: return ImGuiKey_J;
		case Key::K: return ImGuiKey_K;
		case Key::L: return ImGuiKey_L;
		case Key::M: return ImGuiKey_M;
		case Key::N: return ImGuiKey_N;
		case Key::O: return ImGuiKey_O;
		case Key::P: return ImGuiKey_P;
		case Key::Q: return ImGuiKey_Q;
		case Key::R: return ImGuiKey_R;
		case Key::S: return ImGuiKey_S;
		case Key::T: return ImGuiKey_T;
		case Key::U: return ImGuiKey_U;
		case Key::V: return ImGuiKey_V;
		case Key::W: return ImGuiKey_W;
		case Key::X: return ImGuiKey_X;
		case Key::Y: return ImGuiKey_Y;
		case Key::Z: return ImGuiKey_Z;

		// Function keys (ImGui exposes F1-F12 only)
		case Key::F1: return ImGuiKey_F1;
		case Key::F2: return ImGuiKey_F2;
		case Key::F3: return ImGuiKey_F3;
		case Key::F4: return ImGuiKey_F4;
		case Key::F5: return ImGuiKey_F5;
		case Key::F6: return ImGuiKey_F6;
		case Key::F7: return ImGuiKey_F7;
		case Key::F8: return ImGuiKey_F8;
		case Key::F9: return ImGuiKey_F9;
		case Key::F10: return ImGuiKey_F10;
		case Key::F11: return ImGuiKey_F11;
		case Key::F12: return ImGuiKey_F12;

		// Keypad
		case Key::KP0: return ImGuiKey_Keypad0;
		case Key::KP1: return ImGuiKey_Keypad1;
		case Key::KP2: return ImGuiKey_Keypad2;
		case Key::KP3: return ImGuiKey_Keypad3;
		case Key::KP4: return ImGuiKey_Keypad4;
		case Key::KP5: return ImGuiKey_Keypad5;
		case Key::KP6: return ImGuiKey_Keypad6;
		case Key::KP7: return ImGuiKey_Keypad7;
		case Key::KP8: return ImGuiKey_Keypad8;
		case Key::KP9: return ImGuiKey_Keypad9;
		case Key::KPDecimal: return ImGuiKey_KeypadDecimal;
		case Key::KPDivide: return ImGuiKey_KeypadDivide;
		case Key::KPMultiply: return ImGuiKey_KeypadMultiply;
		case Key::KPSubtract: return ImGuiKey_KeypadSubtract;
		case Key::KPAdd: return ImGuiKey_KeypadAdd;
		case Key::KPEqual: return ImGuiKey_KeypadEqual;

		// Modifier keys (also forwarded as ImGuiMod_* each frame - see BeginFrame)
		case Key::LeftShift: return ImGuiKey_LeftShift;
		case Key::LeftControl: return ImGuiKey_LeftCtrl;
		case Key::LeftAlt: return ImGuiKey_LeftAlt;
		case Key::LeftSuper: return ImGuiKey_LeftSuper;
		case Key::RightShift: return ImGuiKey_RightShift;
		case Key::RightControl: return ImGuiKey_RightCtrl;
		case Key::RightAlt: return ImGuiKey_RightAlt;
		case Key::RightSuper: return ImGuiKey_RightSuper;

		// World1, World2, F13-F25 have no ImGui equivalent
		default: return ImGuiKey_None;
		}
	}

	void ImGuiInputCapture(Aether::QueryRes& res)
	{
		LM_PROFILE_FUNCTION();

		auto world = res.World();

		auto& keyboard = world.GetResource<KeyboardState>();
		auto& mouse = world.GetResource<MouseState>();
		auto& io_state = world.GetResourceMut<Glyph::ImGuiIOState>();
		const TimeStep dt = res.DeltaTime();

		auto& io = ImGui::GetIO();

		// 1. Frame Timing
		io.DeltaTime = dt.GetSeconds();

		// 2. Mouse
		io.AddMousePosEvent(mouse.X, mouse.Y);
		io.AddMouseWheelEvent(mouse.ScrollX, mouse.ScrollY);
		for (size_t i = 0; i < Mouse::MaxButtonCode; i++)
		{
			if (mouse.JustPressed[i])
				io.AddMouseButtonEvent(static_cast<int>(i), true);
			if (mouse.JustReleased[i])
				io.AddMouseButtonEvent(static_cast<int>(i), false);
		}

		// 3. Modifier keys - forward current held state every frame so ImGui
		//    sees Ctrl/Shift/Alt even if they were already down before the window gained focus
		io.AddKeyEvent(ImGuiMod_Shift, keyboard.Down(Key::LeftShift) || keyboard.Down(Key::RightShift));
		io.AddKeyEvent(ImGuiMod_Ctrl, keyboard.Down(Key::LeftControl) || keyboard.Down(Key::RightControl));
		io.AddKeyEvent(ImGuiMod_Alt, keyboard.Down(Key::LeftAlt) || keyboard.Down(Key::RightAlt));
		io.AddKeyEvent(ImGuiMod_Super, keyboard.Down(Key::LeftSuper) || keyboard.Down(Key::RightSuper));

		//4. Per key events - only iterate keys that actually changed this frame
		for (KeyCode k = 0; k < Key::MaxKeyCode; k++)
		{
			if (!keyboard.JustPressed[k] && !keyboard.JustReleased[k])
				continue;

			const ImGuiKey imgui_key = KeyCodeToImGuiKey(k);
			if (imgui_key == ImGuiKey_None)
				continue; // Skip keys that don't have an ImGui mapping

			if (keyboard.JustPressed[k])
				io.AddKeyEvent(imgui_key, true);
			if (keyboard.JustReleased[k])
				io.AddKeyEvent(imgui_key, false);
		}

		// 5. Mirror WantCapture flags to the ECS resource so gameplay systems
		//    can check them without touching ImGui directly.
		io_state.WantsCaptureMouse = io.WantCaptureMouse;
		io_state.WantsCaptureKeyboard = io.WantCaptureKeyboard;
		io_state.WantsTextInput = io.WantTextInput;
	}
}

namespace Lumora::Glyph
{
	ImGuiPlugin::ImGuiPlugin(const ImGuiSettings& settings)
		: m_InitialSettings(settings) {}

	void ImGuiPlugin::Build(Core::Application& app)
	{
		LM_PROFILE_FUNCTION();

		auto& world = app.GetWorld();

		/// 1. Create and activate the ImGui context
		m_ImGuiContext = ImGui::CreateContext();
		ImGui::SetCurrentContext(m_ImGuiContext);

		/// 2. Apply engine-side settings to ImGui IO
		ImGuiIO& io = ImGui::GetIO();
		(void)io; // Silence unused variable warning if no settings are applied
		io.IniFilename = m_InitialSettings.IniFilename;

		ImGuiConfigFlags config_flags = 0;
		if (m_InitialSettings.DockingEnabled)
			config_flags |= ImGuiConfigFlags_DockingEnable;
		if (m_InitialSettings.ViewportsEnabled)
			config_flags |= ImGuiConfigFlags_ViewportsEnable;
		io.ConfigFlags |= config_flags;

		/// 3. Expose config and state to the world
		world.SetResource(m_InitialSettings); // Read by tools/editor to know active config
		world.SetResource(ImGuiIOState{});    // Read by input system to know if ImGui wants to capture mouse/keyboard/text input

		/// 4. Renderer Backend
		const auto& window_res = world.GetResource<WindowResource>();
		const auto& render_device_res = world.GetResource<Lumen::RenderDeviceResource>();

		// Set window size
		const auto& window_props = window_res.Resource->GetProps();
		io.DisplaySize = ImVec2(static_cast<float>(window_props.Width), static_cast<float>(window_props.Height));
		// Render Backend
		m_ImGuiBackend = ImGuiBackend::Create(render_device_res.Resource->GetAPI());
		m_ImGuiBackend->Init(*window_res.Resource);

		/// 5. Input handling
		auto input_system = world.System("ImGui::InputCaptureSystem");
		input_system.Read<KeyboardState>().Write<ImGuiIOState>().Read<MouseState>().SetPhase<Aether::Phases::Input>();
		input_system.Run(ImGuiInputCapture);

		/// 6. BeginFrame and EndFrame systems
		auto begin_frame_system = world.System("ImGui::BeginFrameSystem");
		begin_frame_system.SetPhase<Aether::Phases::OnUI>().Read<ImGuiSettings>();
		begin_frame_system.Run([this](Aether::QueryRes& res)
		{
			LM_PROFILE_SCOPE("ImGuiPlugin::BeginFrameSystem");

			m_ImGuiBackend->NewFrame();
			ImGui::NewFrame();

			const auto& settings = res.World().GetResource<ImGuiSettings>();
			if (settings.ShowDemoWindow)
				ImGui::ShowDemoWindow();
		});

		auto end_frame_system = world.System("ImGui::EndFrameSystem");
		end_frame_system.SetPhase<Aether::Phases::PostUI>();
		end_frame_system.Run([this](Aether::QueryRes&)
		{
			LM_PROFILE_SCOPE("ImGuiPlugin::EndFrameSystem");
			ImGui::Render();
			m_ImGuiBackend->RenderDrawData(ImGui::GetDrawData());

			// Update and Render additional Platform Windows
			ImGuiIO& io = ImGui::GetIO();
			if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
			{
				ImGui::UpdatePlatformWindows();
				ImGui::RenderPlatformWindowsDefault();
			}
		});
	}
	void ImGuiPlugin::Finish(Core::Application&) {}
	void ImGuiPlugin::Cleanup(Core::Application& app)
	{
		LM_PROFILE_FUNCTION();

		if (m_ImGuiBackend)
		{
			m_ImGuiBackend->Shutdown();
			m_ImGuiBackend.reset();
		}
		if (m_ImGuiContext)
		{
			ImGui::DestroyContext(m_ImGuiContext);
			m_ImGuiContext = nullptr;
		}
	}
	void ImGuiPlugin::AddDependencies(Core::DependencyList& dependencies)
	{
		dependencies.Require<Flux::WindowPlugin>();
		dependencies.Require<Lumen::RendererPlugin>();
	}
}
