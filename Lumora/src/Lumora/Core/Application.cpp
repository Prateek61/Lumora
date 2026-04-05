#include "LMPCH.h"
#include "Application.h"

#include "Lumora/Core/CoreResource.h"
#include "Lumora/Utilities/Time.h"

namespace Lumora::Core
{
	Application Application::Create()
	{
		return {};
	}

	void Application::Run()
	{

		{
			LM_PROFILE_SCOPE("Plugin Initialization");

			BuildPlugins();
			FinishPlugins();
		}

		LM_PROFILE_END_SESSION();

		LM_PROFILE_BEGIN_SESSION("Runtime", "Runtime.json");

		RunLoop();

		LM_PROFILE_END_SESSION();

		LM_PROFILE_BEGIN_SESSION("Shutdown", "Shutdown.json");

		CleanupPlugins();

		LM_PROFILE_END_SESSION();
	}

	Application::Application()
	{
		Log::Init();
		LM_PROFILE_BEGIN_SESSION("Startup", "Startup.json");

		// Initialize the time system
		auto _ = Time::Get();

		// Register default pipeline phases
		Aether::Phases::Register(m_World);

		// Insert core resources
		m_World.SetResource(ApplicationState{.Running = true});
		m_World.SetResource(DeltaTime{0.0f});

		LM_CORE_INFO("Application created - ECS world initialized");
	}

	void Application::BuildPlugins()
	{
		LM_PROFILE_FUNCTION();

		for (auto& plugin : m_Plugins)
		{
			LM_CORE_INFO("Building plugin '{}'", plugin->GetName());
			plugin->Build(*this);
		}
	}

	void Application::FinishPlugins()
	{
		LM_PROFILE_FUNCTION();

		for (auto& plugin : m_Plugins)
		{
			LM_CORE_INFO("Finishing plugin '{}'", plugin->GetName());
			plugin->Finish(*this);
		}
	}

	void Application::CleanupPlugins()
	{
		LM_PROFILE_FUNCTION();

		for (auto it = m_Plugins.rbegin(); it != m_Plugins.rend(); ++it)
		{
			LM_CORE_INFO("Cleaning up plugin '{}'", (*it)->GetName());
			(*it)->Cleanup(*this);
		}
	}

	void Application::RunLoop()
	{
		auto last_time = std::chrono::steady_clock::now();

		bool running = true;
		while (running)
		{
			LM_PROFILE_SCOPE("Main Loop");

			const auto current_time = std::chrono::steady_clock::now();
			float delta_seconds = std::chrono::duration<float>(current_time - last_time).count();
			delta_seconds = std::min(delta_seconds, 0.1f); // Clamp to avoid large jumps
			last_time = current_time;

			m_World.GetResourceMut<DeltaTime>() = DeltaTime{ delta_seconds };

			// Run the ECS world for one frame.
			running = m_World.Raw().progress(delta_seconds);

			// Update the application state
			m_World.GetResourceMut<ApplicationState>().Running = running;
		}
	}
}
