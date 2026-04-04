#pragma once

#include "Lumora/Aether/Base.h"

namespace Lumora::Aether
{
	template<typename Phase>
	class PhaseBuilder
	{
	public:
		explicit PhaseBuilder(World& world)
			: m_World(&world) {}

		template <typename Dependency>
		PhaseBuilder& DependsOn();

		template <typename Target>
		PhaseBuilder& Before();

	private:
		World* m_World;
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
	template<typename Phase>
	template<typename Dependency>
	PhaseBuilder<Phase>& PhaseBuilder<Phase>::DependsOn()
	{
		m_World->Raw().component<Phase>()
			.template depends_on<Dependency>();
		return *this;
	}

	template<typename Phase>
	template<typename Target>
	PhaseBuilder<Phase>& PhaseBuilder<Phase>::Before()
	{
		m_World->Raw().component<Phase>()
			.template before<Target>();
		return *this;
	}
}