#pragma once

#include <cstdint>
#include <functional>

#include "Lumora/Flux/WindowProps.h"
#include "Lumora/Flux/RawEvents.h"

// Forward declare — Window.cpp includes <GLFW/glfw3.h>, nothing else does.
struct GLFWwindow;

namespace Lumora::Flux
{
	class Window
	{
	public:
		explicit Window(const WindowProps& props);
		~Window();

		// Non-copyable, movable
		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;
		Window(Window&& other) noexcept;
		Window& operator=(Window&& other) noexcept;

		// Accessors
		const WindowProps& GetProps() const { return m_Props; }

		//Raw Handles
		void* GetGLFWHandle() const { return m_GLFWWindow; }
		void* GetNativeHandle() const { return m_NativeWindow; }

		// Operations
		void PollEvents();
		void SetTitle(const std::string& title);

		// These are called by the window plugin to sync state
		void UpdateSize(uint32_t width, uint32_t height);
		
		void SetupCallback(const std::function<void(const Raw::RawEvent&)>& callbackFn);
	private:
		GLFWwindow* m_GLFWWindow = nullptr;
		void* m_NativeWindow = nullptr;

		WindowProps m_Props;
		std::function<void(const Raw::RawEvent&)> m_EventCallback;

		void Init(const WindowProps& props);
		void Shutdown();
		void AdoptUserPointer();
	};
}