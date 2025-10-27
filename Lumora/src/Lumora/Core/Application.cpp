#include "LMPCH.h"
#include "Application.h"

#include "Lumora/Utilities/Time.h"
#include "Lumora/Renderer/Renderer.h"

namespace
{
	Lumora::Application* s_Instance = nullptr;
}

namespace Lumora
{
	Application& Application::Get()
	{
		return *s_Instance;
	}

	Application::Application(const std::filesystem::path& configFile, ApplicationCommandLineArgs args)
	{
		LM_PROFILE_FUNCTION();

		auto props = ApplicationProps::Get(configFile, m_Serializer);

		Init(props);
	}

	Application::Application(const ApplicationProps& props)
	{
		LM_PROFILE_FUNCTION();

		Init(props);
	}

	Application::~Application()
	{
		m_LayerStack.~LayerStack();

		Renderer::Shutdown();
	}

	void Application::Init(const ApplicationProps& props)
	{
		LM_PROFILE_FUNCTION();

		LM_CORE_DEBUG("Initializing Application with props\n {}", m_Serializer.SerializeToLuaScript<ApplicationProps>(props));

		LM_CORE_ASSERT(!s_Instance, "Applciation already exists")
		s_Instance = this;

		m_Props = props;


		// Window
		m_Window = CreateScope<Window>(props.WindowProps);
		m_Window->SetEventCallback(LM_BIND_EVENT_FN(Application::OnEvent));

		m_Running = props.Run;

		// Renderer context
		Renderer::SetAPI(props.API);
		Renderer::Init(*m_Window);

		// Asset Manager
		m_AssetManager = CreateScope<AssetManager>(props.AssetsDirectory, m_Serializer);

		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);
	}

	void Application::OnEvent(Event& e)
	{
		LM_PROFILE_FUNCTION();

		EventDispatcher dispatcher(e);

		// Window Functions
		dispatcher.Dispatch<WindowCloseEvent>(LM_BIND_EVENT_FN(Application::OnWindowClose));
		dispatcher.Dispatch<WindowResizeEvent>(LM_BIND_EVENT_FN(Application::OnWindowResize));

		// TODO: Dispatch events to layers
		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin(); )
		{
			(*--it)->OnEvent(e);
			if (e.Handled)
				break;
		}
	}

	void Application::Close()
	{
		m_Running = false;
	}

	void Application::Run()
	{
		LM_PROFILE_FUNCTION();

		m_LastFrameTime = Time::GetF();

		while( m_Running )
		{
			LM_PROFILE_SCOPE("RunLoop");

			const float time = Time::GetF();
			const TimeStep time_step = time - m_LastFrameTime;
			m_LastFrameTime = time;

			

			{
				LM_PROFILE_SCOPE("LayerStack OnUpdate");

				for (Layer* layer : m_LayerStack)
					layer->OnUpdate(time_step);
			}

			{
				LM_PROFILE_SCOPE("LayerStack OnRender");

				Renderer::BeginFrame();

				OnUpdate(time_step);

				for (Layer* layer : m_LayerStack)
					layer->OnRender();

				Renderer::EndFrame();
			}

			{
				LM_PROFILE_SCOPE("LayerStack OnImGuiRender");

				m_ImGuiLayer->BeginImGuiFrame();

				for (Layer* layer : m_LayerStack)
					layer->OnImGuiRender(time_step);

				m_ImGuiLayer->EndImGuiFrame();
			}

			m_Window->OnUpdate();
		}
	}

	void Application::PushLayer(Layer* layer)
	{
		LM_PROFILE_FUNCTION();
		LM_CORE_ASSERT(layer, "Layer is nullptr!")

		LM_CORE_TRACE("Pushing Layer ({})", layer->GetName());

		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}


	void Application::PushOverlay(Layer* overlay)
	{
		LM_PROFILE_FUNCTION();
		LM_CORE_ASSERT(overlay, "Overlay is nullptr!")

		LM_CORE_TRACE("Pushing Overlay ({})", overlay->GetName());

		m_LayerStack.PushOverlay(overlay);
		overlay->OnAttach();
	}


	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		m_Running = false;
		return true;
	}

	bool Application::OnWindowResize(WindowResizeEvent& e)
	{
		LM_PROFILE_FUNCTION();

		if (e.GetWidth() == 0 || e.GetHeight() == 0)
			return false;
		Renderer::Resize(e.GetWidth(), e.GetHeight());

		return false;
	}
}
