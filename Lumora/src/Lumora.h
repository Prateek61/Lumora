#pragma once

#include <iostream>

// Asset
#include "Lumora/Asset/AssetCommon.h"
#include "Lumora/Asset/Asset.h"
#include "Lumora/Asset/AssetProps.h"
#include "Lumora/Asset/AssetHandle.h"
#include "Lumora/Asset/AssetTypeRegistry.h"
#include "Lumora/Asset/AssetRegistry.h"
#include "Lumora/Asset/AssetStorage.h"
#include "Lumora/Asset/AssetManager.h"
#include "Lumora/Asset/Assets.h"
#include "Lumora/Asset/AssetReloader.h"

// Common
#include "Lumora/Common/UUID.h"
#include "Lumora/Common/Log.h"
#include "Lumora/Common/Assert.h"
#include "Lumora/Common/Instrumentor.h"

// Core
#include "Lumora/Core/Window.h"
#include "Lumora/Core/Props.h"
#include "Lumora/Core/Application.h"

// Event
#include "Lumora/Event/Event.h"
#include "Lumora/Event/ApplicationEvent.h"
#include "Lumora/Event/KeyEvent.h"
#include "Lumora/Event/MouseEvent.h"

// Scripting
#include "Lumora/Scripting/LuaSerializer.h"

// Renderer
#include "Lumora/Renderer/RendererContext.h"

// Utilities
#include "Lumora/Utilities/Time.h"
#include "Lumora/Utilities/TimeStep.h"
#include "Lumora/Utilities/FileDialog.h"

namespace Lm = Lumora;