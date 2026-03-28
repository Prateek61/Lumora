#pragma once

#include "Lumora/Common/Base.h"
#include <flecs.h>

namespace Lumora
{
    class App
    {
    public:
        App()
        {
			flecs::world world;
            LM_CORE_INFO("App created");
        }
    };
}
