#include "LMPCH.h"
#include "World.h"

namespace Lumora::Aether
{
	World::World() = default;
	World::~World() = default;

	Entity World::CreateEntity(const char* name)
	{
		LM_PROFILE_FUNCTION();

		if (name)
			return Entity(m_World.entity(name));

		return Entity(m_World.entity());
	}
}