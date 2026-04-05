#pragma once

#include <vector>
#include <typeindex>
#include <unordered_set>

#include "Lumora/Core/Base.h"
#include "Lumora/Aether/World.h"

namespace Lumora::Core
{
    // Forward Declaration
    class Plugin;

	/**
	 The App class is the main entry point of the application. It manages the lifecycle of the application, including initialization, running, and shutdown. It also holds references to plugins and the ECS world.
     */
    class Application
    {
    public:
	    /**
		 * Creates a new instance of the App class. This method initializes the application and prepares it for running.
		 * @return A new instance of the App class, ready to be run.
         */
        static Application Create();

        // Add a plugin
        template <typename T, typename... Args>
        Application& AddPlugin(Args&&... args);

        template <typename T>
        Application& InsertResource(T&& value);

        // Lifecycles
        void Run();
        void Quit();

        // Accessors
		Aether::World& GetWorld() { return m_World; }
		const Aether::World& GetWorld() const { return m_World; }

		// Application is move-only — vector<unique_ptr<Plugin>> can't be copied.
		Application(Application&&) noexcept = default;
		Application& operator=(Application&&) = default;
		Application(const Application&) = delete;
		Application& operator=(const Application&) = delete;

    private:
        Aether::World m_World;
        std::vector<Scope<Plugin>> m_Plugins;
		std::unordered_set<std::type_index> m_RegisteredPlugins;

        Application();

        void BuildPlugins();
        void FinishPlugins();
        void CleanupPlugins();
        void RunLoop();
    };
}

#include "Lumora/Core/Plugin.h"

// Template Implementations
namespace Lumora::Core
{
	template <typename T, typename... Args>
    Application& Application::AddPlugin(Args&&... args)
    {
        static_assert(std::derived_from<T, Plugin>, "T must derive from Plugin");
        LM_PROFILE_FUNCTION();

        std::type_index typeIndex(typeid(T));
        if (m_RegisteredPlugins.contains(typeIndex))
        {
            LM_CORE_ERROR("Plugin '{}' is already registered!", typeid(T).name());
            return *this;
        }

		m_RegisteredPlugins.insert(typeIndex);
		auto& plugin = m_Plugins.emplace_back(CreateScope<T>(std::forward<Args>(args)...));
		LM_CORE_INFO("Added plugin '{}'", plugin->GetName());
        return *this;
	}

    template<typename T>
    Application& Application::InsertResource(T&& value)
    {
		LM_PROFILE_FUNCTION();

        m_World.SetResource(std::forward<T>(value));
		return *this;
    }
}
