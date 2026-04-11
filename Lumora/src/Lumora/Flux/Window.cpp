#include "LMPCH.h"
#include "Window.h"

#include "GLFW/glfw3.h"
#include "GLFW/glfw3native.h"

namespace
{
	uint8_t s_GLFWWindowCount = 0;

	void GLFWErrorCallback(int error, const char* description)
	{
		LM_CORE_ERROR("GLFW Error ({0}): {1}", error, description);
	}
}

namespace Lumora::Flux
{
	Window::Window(const WindowProps& props)
		: m_Props(props)
	{
		Init(props);
	}

	Window::~Window()
	{
		Shutdown();
	}

	Window::Window(Window&& other) noexcept
		: m_GLFWWindow(other.m_GLFWWindow), m_NativeWindow(other.m_NativeWindow), m_Props(other.m_Props), m_EventCallback(std::move(other.m_EventCallback))
	{
		m_GLFWWindow = nullptr;
		m_NativeWindow = nullptr;
	}

	Window& Window::operator=(Window&& other) noexcept
	{
		if (this != &other)
		{
			Shutdown();
			m_EventCallback = std::move(other.m_EventCallback);
			m_GLFWWindow = other.m_GLFWWindow;
			m_NativeWindow = other.m_NativeWindow;
			m_Props = other.m_Props;
			other.m_GLFWWindow = nullptr;
			other.m_NativeWindow = nullptr;
		}
		return *this;
	}

	void Window::PollEvents()
	{
		glfwPollEvents();
	}

	void Window::SetTitle(const std::string& title)
	{
		glfwSetWindowTitle(m_GLFWWindow, title.c_str());
	}

	void Window::UpdateSize(uint32_t width, uint32_t height)
	{
		// This is called by the window plugin when it receives a resize event from GLFW. We update our cached size here so that GetProps() always returns the correct size.
		m_Props.Width = width;
		m_Props.Height = height;
	}

	void Window::SetupCallback(const std::function<void(const Raw::RawEvent&)>& callbackFn)
	{
		LM_PROFILE_FUNCTION();

		m_EventCallback = callbackFn;

		// Window Callbacks
		glfwSetWindowSizeCallback(m_GLFWWindow, [](GLFWwindow* window, int width, int height)
		{
			LM_PROFILE_FUNCTION();

			auto win = static_cast<Window*>(glfwGetWindowUserPointer(window));
			Raw::WindowResize event{static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
			win->m_EventCallback(event);
		});
		glfwSetWindowCloseCallback(m_GLFWWindow, [](GLFWwindow* window)
		{
			LM_PROFILE_FUNCTION();
			auto win = static_cast<Window*>(glfwGetWindowUserPointer(window));
			Raw::WindowClose event{};
			win->m_EventCallback(event);
		});
		glfwSetWindowFocusCallback(m_GLFWWindow, [](GLFWwindow* window, int focused)
		{
			LM_PROFILE_FUNCTION();
			auto win = static_cast<Window*>(glfwGetWindowUserPointer(window));
			Raw::WindowFocus event{focused == GLFW_TRUE};
			win->m_EventCallback(event);
		});

		// Key Callbacks
		glfwSetKeyCallback(m_GLFWWindow, [](GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			LM_PROFILE_FUNCTION();
			auto win = static_cast<Window*>(glfwGetWindowUserPointer(window));
			Raw::KeyAction event{static_cast<KeyCode>(key), action, mods};
			win->m_EventCallback(event);
		});
		glfwSetCharCallback(m_GLFWWindow, [](GLFWwindow* window, unsigned int codepoint)
		{
			LM_PROFILE_FUNCTION();
			auto win = static_cast<Window*>(glfwGetWindowUserPointer(window));
			Raw::CharTyped event{codepoint};
			win->m_EventCallback(event);
		});

		// Mouse Callbacks
		glfwSetMouseButtonCallback(m_GLFWWindow, [](GLFWwindow* window, int button, int action, int mods)
		{
			LM_PROFILE_FUNCTION();
			auto win = static_cast<Window*>(glfwGetWindowUserPointer(window));
			Raw::MouseButton event{static_cast<MouseCode>(button), action, mods};
			win->m_EventCallback(event);
		});
		glfwSetCursorPosCallback(m_GLFWWindow, [](GLFWwindow* window, double xpos, double ypos)
		{
			LM_PROFILE_FUNCTION();
			auto win = static_cast<Window*>(glfwGetWindowUserPointer(window));
			Raw::MouseMove event{static_cast<float>(xpos), static_cast<float>(ypos)};
			win->m_EventCallback(event);
		});
		glfwSetScrollCallback(m_GLFWWindow, [](GLFWwindow* window, double xoffset, double yoffset)
		{
			LM_PROFILE_FUNCTION();
			auto win = static_cast<Window*>(glfwGetWindowUserPointer(window));
			Raw::MouseScroll event{static_cast<float>(xoffset), static_cast<float>(yoffset)};
			win->m_EventCallback(event);
		});
	}

	void Window::Init(const WindowProps& props)
	{
		LM_PROFILE_FUNCTION();

		LM_CORE_INFO("Creating window: {0} ({1}x{2})", props.Title, props.Width, props.Height);

		if (s_GLFWWindowCount == 0)
		{
			LM_PROFILE_SCOPE("Window::Init glfwInit");

			int success = glfwInit();
			LM_CORE_ASSERT(success, "Could not initialize GLFW!");
			glfwSetErrorCallback(GLFWErrorCallback);
		}

		{
			LM_PROFILE_SCOPE("Window::Init glfwCreateWindow");

			DEBUG_ONLY
			(
				glfwWindowHint(GLFW_CONTEXT_DEBUG, GLFW_TRUE);
			)
			glfwWindowHint(GLFW_CLIENT_API, props.API == Lumen::RenderAPI::OpenGL ? GLFW_OPENGL_API : GLFW_NO_API);
			m_GLFWWindow = glfwCreateWindow(static_cast<int>(props.Width), static_cast<int>(props.Height),
			                                props.Title.c_str(), nullptr, nullptr);

			++s_GLFWWindowCount;
		}

		glfwSetWindowUserPointer(m_GLFWWindow, this);

		// Native Windows
#ifdef LM_PLATFORM_WINDOWS
		m_NativeWindow = glfwGetWin32Window(m_GLFWWindow);
#elif defined(LM_PLATFORM_LINUX)
		m_NativeWindow = (void*)glfwGetX11Window(m_GLFWWindow);
#elif defined(LM_PLATFORM_MACOS)
		m_NativeWindow = glfwGetCocoaWindow(m_GLFWWindow);
#else
#error "Not implemented! for this platform"
#endif
	}

	void Window::Shutdown()
	{
		LM_PROFILE_FUNCTION();

		if (!m_GLFWWindow)
			return;

		glfwDestroyWindow(m_GLFWWindow);
		--s_GLFWWindowCount;
		m_GLFWWindow = nullptr;
		m_NativeWindow = nullptr;

		if (s_GLFWWindowCount == 0)
		{
			LM_PROFILE_SCOPE("Window::Shutdown glfwTerminate");
			glfwTerminate();
		}
	}
}
