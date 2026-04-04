#pragma once

#include "Lumora/Core/Base.h"

namespace Lumora::Core
{
	// Forward Declaration
	class App;

	class Plugin
	{
	public:
		virtual ~Plugin() = default;

		// Register components, systems, resources and observers
		virtual void Build(App& app) = 0;

		// Called after all plugins have been built
		virtual void Finish(App& app) {}

		// Called on shutdown in reverse order. Release resources and perform cleanup here.
		virtual void Cleanup(App& app) {}

		virtual const char* GetName() const = 0;
	};
}