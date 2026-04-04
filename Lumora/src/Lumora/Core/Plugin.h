#pragma once

namespace Lumora::Core
{
	// Forward Declaration
	class Application;

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

		virtual const char* GetName() const = 0;
	};
}