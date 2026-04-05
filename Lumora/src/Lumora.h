
#pragma once

#include <iostream>

// Core
#include "Lumora/Core/Base.h"
#include "Lumora/Core/Application.h"
#include "Lumora/Core/CoreResource.h"
#include "Lumora/Core/Plugin.h"

// Aether (Entity Component System)
#include "Lumora/Aether/Entity.h"
#include "Lumora/Aether/World.h"
#include "Lumora/Aether/System.h"
#include "Lumora/Aether/Query.h"
#include "Lumora/Aether/Phase.h"

// Flux (Windowing, Input, etc.)
#include "Lumora/Flux/FluxPlugin.h"
#include "Lumora/Flux/Window.h"
#include "Lumora/Flux/WindowProps.h"
#include "Lumora/Flux/Input.h"

namespace Lm = Lumora;