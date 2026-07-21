#include "LMPCH.h"
#include "World.h"

namespace Lumora::Aether
{
	World::World() = default;
	World::World(flecs::world flecsWorld)
		: m_World(std::move(flecsWorld)) {}
	World::~World() = default;

	Entity World::CreateEntity(const char* name)
	{
		LM_PROFILE_FUNCTION();

		return Entity(m_World.entity(name));
	}
	Entity World::CreateEntity()
	{
		LM_PROFILE_FUNCTION();

		return Entity(m_World.entity());
	}
	void World::Quit() const
	{
		LM_PROFILE_FUNCTION();

		m_World.quit();
	}
	bool World::Progress(TimeStep deltaTime) const
	{
		LM_PROFILE_FUNCTION();

		return m_World.progress(deltaTime.GetSeconds());
	}
}