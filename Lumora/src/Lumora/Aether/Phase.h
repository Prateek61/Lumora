#pragma once

#include "Lumora/Aether/Base.h"

namespace Lumora::Aether
{
	class PhaseBuilder
	{
	public:
		explicit PhaseBuilder(World& world, Entity phase);

		template <typename Dependency>
		PhaseBuilder& DependsOn();
		PhaseBuilder& DependsOn(Entity dependency);

		template <typename Target>
		PhaseBuilder& Before();
		PhaseBuilder& Before(Entity target);

	private:
		World* m_World;
		flecs::entity m_PhaseEntity;
	};

	namespace Phases
	{
		struct OnLoad {};

		struct PostLoad {};

		struct PreUpdate {};

		struct OnUpdate {};

		struct OnValidate {};

		struct PostUpdate {};

		struct PreStore {};

		struct OnStore {};

		void Register(World& world);
	}
}

#include "Lumora/Aether/World.h"

// Template Implementation
namespace Lumora::Aether
{
	template <typename Dependency>
	PhaseBuilder& PhaseBuilder::DependsOn()
	{
		auto dependency_entity = m_World->Raw().entity<Dependency>();

		LM_CORE_ASSERT(dependency_entity.is_valid() && dependency_entity.has(flecs::Phase),
		               "Dependency phase does not exist or is not a phase.");

		m_PhaseEntity.depends_on(dependency_entity);
		return *this;
	}

	template <typename Target>
	PhaseBuilder& PhaseBuilder::Before()
	{
		auto target_entity = m_World->Raw().entity<Target>();

		LM_CORE_ASSERT(target_entity.is_valid() && target_entity.has(flecs::Phase), "Target phase does not exist or is not a phase.");

		target_entity.depends_on(m_PhaseEntity);
		return *this;
	}
}
