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
	template<typename Dependency>
	PhaseBuilder& PhaseBuilder::DependsOn()
	{
		m_PhaseEntity.depends_on(m_World->Raw().entity<Dependency>());
	}

	template<typename Target>
	PhaseBuilder& PhaseBuilder::Before()
	{
		m_World->Raw().entity<Target>().depends_on(m_PhaseEntity);
	}
}