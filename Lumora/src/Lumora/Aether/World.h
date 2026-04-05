#pragma once

#include "Lumora/Aether/Base.h"
#include "Lumora/Aether/Entity.h"

#include "Lumora/Core/Instrumentor.h"

namespace Lumora::Aether
{
	class World
	{
	public:
		explicit World();
		explicit World(flecs::world flecsWorld);
		~World();

		// Entity Management
		Entity CreateEntity(const char* name = nullptr);

		// Resources (singleton)
		template <typename T>
		void SetResource(T&& value);

		template <typename T>
		const T* TryGetResource() const;

		template <typename T>
		const T& GetResource() const;

		template <typename T>
		T* TryGetResourceMut();

		template <typename T>
		T& GetResourceMut();

		// Phases
		template <typename Phase>
		PhaseBuilder<Phase> AddPhase();

		template <typename Phase, typename DependsOnPhase>
		void AddPhaseAfter();

		// TODO: Systems
		// TODO: Observers

		// Raw access
		flecs::world& Raw() { return m_World; }
		const flecs::world& Raw() const { return m_World; }

	private:
		flecs::world m_World;
	};
}

// Template Implementations
#include "Lumora/Aether/Phase.h"

namespace Lumora::Aether
{
	template <typename T>
	void World::SetResource(T&& value)
	{
		LM_PROFILE_FUNCTION();

		m_World.set<T>(std::forward<T>(value));
	}

	template <typename T>
	const T* World::TryGetResource() const
	{
		LM_PROFILE_FUNCTION();

		return m_World.try_get<T>();
	}

	template <typename T>
	const T& World::GetResource() const
	{
		LM_PROFILE_FUNCTION();

		return m_World.get<T>();
	}

	template<typename T>
	T* World::TryGetResourceMut()
	{
		LM_PROFILE_FUNCTION();

		return m_World.try_get_mut<T>();
	}

	template<typename T>
	inline T& World::GetResourceMut()
	{
		LM_PROFILE_FUNCTION();

		return m_World.get_mut<T>();
	}

	template <typename Phase>
	PhaseBuilder<Phase> World::AddPhase()
	{
		LM_PROFILE_FUNCTION();

		m_World.component<Phase>()
			.add(flecs::Phase);
		return PhaseBuilder<Phase>(*this);
	}

	template <typename Phase, typename DependsOnPhase>
	void World::AddPhaseAfter()
	{
		LM_PROFILE_FUNCTION();

		m_World.component<Phase>()
			.add(flecs::Phase)
			.template before<DependsOnPhase>();
	}
}
