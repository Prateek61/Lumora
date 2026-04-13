#pragma once

#include <vector>
#include <typeindex>

namespace Lumora::Core
{
	// Forward Declaration
	class Application;
	class DependencyList;

	class Plugin
	{
	public:
		virtual ~Plugin() = default;

		// Register components, systems, resources and observers
		virtual void Build(Application& app) = 0;

		// Called after all plugins have been built
		virtual void Finish(Application& app) {}

		// Called on shutdown in reverse order. Release resources and perform cleanup here.
		virtual void Cleanup(Application& app) {}

		// Adds dependencies to the plugin. Used for checking if all required plugins are present before building.
		virtual void AddDependencies(DependencyList& dependencies) {}

		virtual const char* GetName() const = 0;
	};

	class DependencyList
	{
	public:
		template <typename T>
		void Require() { m_RequiredPlugins.push_back(typeid(T)); }

		const auto& GetRequiredPlugins() const { return m_RequiredPlugins; }
	private:
		std::vector<std::type_index> m_RequiredPlugins;
	};
}