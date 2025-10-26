#pragma once

#include "Lumora/Common/Base.h"
#include "Lumora/Core/Props.h"
#include "Lumora/Event/Event.h"

namespace Lumora
{
	class Window
	{
	public:
		using EventCallbackFn = std::function<void(Event&)>;

		explicit Window(const WindowProps& props);
		~Window();

		void OnUpdate();

		uint32_t GetWidth() const { return m_Props.Width; }
		uint32_t GetHeight() const { return m_Props.Height; }
		const WindowProps& GetProps() const { return m_Props; }

		void* GetGLFWWindow() { return m_GLFWWindow; }
		void* GetNativeWindow() const { return m_NativeWindow; }

		void SetEventCallback(const EventCallbackFn& callback) { m_EventCallback = callback; }
		void EventCallback(Event& e) const { m_EventCallback(e); }

	private:
		WindowProps m_Props;
		void* m_GLFWWindow = nullptr;
		void* m_NativeWindow = nullptr; // Platform specific window handle
		EventCallbackFn m_EventCallback;

	private:
		void Init(const WindowProps& props);
		void SetGLFWCallbacks();
		void Shutdown();
	};
}