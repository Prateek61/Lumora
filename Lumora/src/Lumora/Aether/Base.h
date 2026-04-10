#pragma once

#include <flecs.h>

namespace Lumora::Aether
{
	// Forward Declarations
	struct Entity;
	class World;
	class System;
	template <typename... Components>
	class Query;
	class PhaseBuilder;
}