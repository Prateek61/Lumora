
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
#include "Lumora/Aether/QueryRes.h"
#include "Lumora/Aether/Phase.h"
#include "Lumora/Aether/FlecsDiagonisticPlugin.h"

// Flux (Windowing, Input, etc.)
#include "Lumora/Flux/WindowPlugin.h"
#include "Lumora/Flux/Window.h"
#include "Lumora/Flux/WindowProps.h"
#include "Lumora/Flux/Input.h"

// Lumen (Rendering)
#include "Lumora/Lumen/RendererPlugin.h"
#include "Lumora/Lumen/RenderDevice.h"
#include "Lumora/Lumen/RenderTypes.h"
#include "Lumora/Lumen/RenderAPI.h"
#include "Lumora/Lumen/Props.h"
#include "Lumora/Lumen/Renderer2D.h"
#include "Lumora/Lumen/Renderer2DPlugin.h"

// UI (Glyph)
#include "Lumora/Glyph/ImGuiPlugin.h"