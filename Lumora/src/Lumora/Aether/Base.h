#pragma once

#include <flecs.h>

namespace Lumora::Aether
{
	// Forward Declarations
	struct Entity;
	class World;
	class System;
	template<typename... Components>
	class SystemBuilder;
	class PhaseBuilder;
	template<typename T>
	struct Field;
	class QueryRes;
}