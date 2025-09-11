#pragma once

#include "Lumora/Common/Base.h"
#include "Lumora/Core/Props.h"
#include "Lumora/Core/Window.h"
#include "Lumora/Renderer/RendererContext.h"
#include "Lumora/Scripting/LuaSerializer.h"
#include "Lumora/Event/ApplicationEvent.h"
#include "Lumora/Utilities/TimeStep.h"

// Forward declaration of main
int main(int argc, char** argv);

namespace Lumora
{
	class Application
	{
	public:
		static Application& Get();

		Application(const std::filesystem::path& configFile, ApplicationCommandLineArgs args);
		Application(const ApplicationProps& props);
		virtual ~Application();

		void OnEvent(Event& e);
		void Close();
		void Run();

		virtual void OnUpdate(TimeStep ts) {} // If user doesn't want to use Layer system

		LuaSerializer& GetSerializer() { return m_Serializer; }
		Window& GetWindow() { return *m_Window; }

	private:
		ApplicationProps m_Props;
		Scope<Window> m_Window;
		Scope<RendererContext> m_RendererContext;

		LuaSerializer m_Serializer = {};
		bool m_Running = true;
		float m_LastFrameTime = 0.0f;

	private:
		void Init(const ApplicationProps& props);
		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);

		friend int ::main(int argc, char** argv);
	};
}