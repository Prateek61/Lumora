#include "LMPCH.h"
#include "Window.h"

#include "GLFW/glfw3.h"

#include "Lumora/Event/ApplicationEvent.h"
#include "Lumora/Event/KeyEvent.h"
#include "Lumora/Event/MouseEvent.h"
#include "GLFW/glfw3native.h"

namespace
{
	uint8_t s_GLFWWindowCount = 0;

	void GLFWErrorCallback(int error, const char* description)
	{
		LM_CORE_ERROR("GLFW Error ({0}): {1}", error, description);
	}
}

namespace Lumora
{
	Window::Window(const WindowProps& props)
	{
		Init(props);
	}

	Window::~Window()
	{
		Shutdown();
	}

	void Window::OnUpdate()
	{
		LM_PROFILE_FUNCTION();

		glfwPollEvents();
	}

	void Window::Init(const WindowProps& props)
	{
		LM_PROFILE_FUNCTION();

		m_Props = props;

		LM_CORE_INFO("Creating window {0} ({1}, {2})", props.Title, props.Width, props.Height);

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
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
			m_GLFWWindow = static_cast<void*>(
				glfwCreateWindow(static_cast<int>(props.Width), static_cast<int>(props.Height), m_Props.Title.c_str(),
				                 nullptr, nullptr)
			);

			++s_GLFWWindowCount;
		}

		glfwSetWindowUserPointer(static_cast<GLFWwindow*>(m_GLFWWindow), this);

		SetGLFWCallbacks();

		// Native Window
#ifdef LM_PLATFORM_WINDOWS
		m_NativeWindow = glfwGetWin32Window(static_cast<GLFWwindow*>(m_GLFWWindow));
#elif defined(LM_PLATFORM_LINUX)
		m_NativeWindow = (void*)glfwGetX11Window(static_cast<GLFWwindow*>(m_GLFWWindow));
#elif defined(LM_PLATFORM_MACOS)
		m_NativeWindow = glfwGetCocoaWindow(static_cast<GLFWwindow*>(m_GLFWWindow));
#else
#error "Not implemented! for this platform"
#endif
	}

	void Window::SetGLFWCallbacks()
	{
		LM_PROFILE_FUNCTION();

		auto window = static_cast<GLFWwindow*>(m_GLFWWindow);

		// Size Callback
		glfwSetWindowSizeCallback(window, [](GLFWwindow* glfwWindow, int width, int height)
		{
			LM_PROFILE_FUNCTION();

			auto window = static_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
			window->m_Props.Width = static_cast<uint32_t>(width);
			window->m_Props.Height = static_cast<uint32_t>(height);

			WindowResizeEvent event(width, height);
			window->EventCallback(event);
		});

		// Close Callback
		glfwSetWindowCloseCallback(window, [](GLFWwindow* glfwWindow)
		{
			LM_PROFILE_FUNCTION();

			auto window = static_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));

			WindowCloseEvent event;
			window->EventCallback(event);
		});

		// Key callbacks
		glfwSetKeyCallback(window, [](GLFWwindow* glfwWindow, int key, int scancode, int action, int mods)
		{
			LM_PROFILE_FUNCTION();

			auto window = static_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));

			switch (action)
			{
			case GLFW_PRESS:
				{
					KeyPressedEvent event(static_cast<uint16_t>(key), false);
					window->EventCallback(event);
					break;
				}
			case GLFW_RELEASE:
				{
					KeyReleasedEvent event(static_cast<uint16_t>(key));
					window->EventCallback(event);
					break;
				}
			case GLFW_REPEAT:
				{
					KeyPressedEvent event(static_cast<uint16_t>(key), true);
					window->EventCallback(event);
					break;
				}
			default:
				{
					break;
				}
			}
		});
		glfwSetCharCallback(window, [](GLFWwindow* glfwWindow, unsigned int keycode)
		{
			LM_PROFILE_FUNCTION();

			auto window = static_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));

			KeyTypedEvent event(static_cast<uint16_t>(keycode));
			window->EventCallback(event);
		});

		// Mouse Callbacks
		glfwSetMouseButtonCallback(window, [](GLFWwindow* glfwWindow, int button, int action, int mods)
		{
			LM_PROFILE_FUNCTION();

			auto window = static_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));

			switch (action)
			{
			case GLFW_PRESS:
				{
					MouseButtonPressedEvent event(button);
					window->EventCallback(event);
					break;
				}
			case GLFW_RELEASE:
				{
					MouseButtonReleasedEvent event(static_cast<uint16_t>(button));
					window->EventCallback(event);
					break;
				}
			default:
				{
					break;
				}
			}
		});
		glfwSetScrollCallback(window, [](GLFWwindow* glfwWindow, double xOffset, double yOffset)
		{
			LM_PROFILE_FUNCTION();

			auto window = static_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));

			MouseScrolledEvent event(static_cast<float>(xOffset), static_cast<float>(yOffset));
			window->EventCallback(event);
		});
		glfwSetCursorPosCallback(window, [](GLFWwindow* glfwWindow, double xPos, double yPos)
		{
			LM_PROFILE_FUNCTION();

			auto window = static_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));

			MouseMovedEvent event(static_cast<float>(xPos), static_cast<float>(yPos));
			window->EventCallback(event);
		});
	}

	void Window::Shutdown()
	{
		LM_PROFILE_FUNCTION();

		glfwDestroyWindow(static_cast<GLFWwindow*>(m_GLFWWindow));
		--s_GLFWWindowCount;

		if (s_GLFWWindowCount == 0)
		{
			LM_PROFILE_SCOPE("Window::Shutdown glfwTerminate");
			glfwTerminate();
		}
	}

	SharedWindow::SharedWindow(const WindowProps& props, void* glfwWindow)
	{
		LM_PROFILE_FUNCTION();

		glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
		m_GLFWWindow = static_cast<void*>(glfwCreateWindow(
			static_cast<int>(props.Width), static_cast<int>(props.Height), props.Title.c_str(),
			nullptr, static_cast<GLFWwindow*>(glfwWindow)
		));

		LM_CORE_TRACE("SharedWindow created: m_GLFWWindow = {0:X}", reinterpret_cast<uintptr_t>(m_GLFWWindow));

		s_GLFWWindowCount++;
	}

	SharedWindow::~SharedWindow()
	{
		LM_PROFILE_FUNCTION();
		LM_CORE_TRACE("Destroying SharedWindow: m_GLFWWindow = {0:X}", reinterpret_cast<uintptr_t>(m_GLFWWindow));

		glfwDestroyWindow(static_cast<GLFWwindow*>(m_GLFWWindow));
		--s_GLFWWindowCount;

		if (s_GLFWWindowCount == 0)
		{
			LM_PROFILE_SCOPE("Window::Shutdown glfwTerminate");
			glfwTerminate();
		}
	}

}
