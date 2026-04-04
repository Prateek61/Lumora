#pragma once

#include "Lumora/Core/Base.h"

namespace Lumora
{
    // Forward Declaration
    class Plugin;

	/**
	 The App class is the main entry point of the application. It manages the lifecycle of the application, including initialization, running, and shutdown. It also holds references to plugins and the ECS world.
     */
    class App
    {
    public:
	    /**
		 * Creates a new instance of the App class. This method initializes the application and prepares it for running.
		 * @return A new instance of the App class, ready to be run.
         */
        static App Create();

        // Add a plugin
    };
}
