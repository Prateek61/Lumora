#pragma once

#include "Lumora/Aether/Base.h"
#include "Lumora/Aether/Entity.h"

#include "Lumora/Core/Instrumentor.h"
#include "Lumora/Utilities/TimeStep.h"

namespace Lumora::Aether
{
	class World
	{
	public:
		explicit World();
		explicit World(flecs::world flecsWorld);
		~World();

		// Entity Management
		Entity CreateEntity(const char* name);
		Entity CreateEntity();

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
		template <typename T>
		void RemoveResource();

		// Lifecycle
		void Quit() const;
		bool Progress(TimeStep deltaTime) const;

		// Phases
		template <typename Phase>
		PhaseBuilder AddPhase();

		// TODO: Systems
		template <typename... Components>
		SystemBuilder<Components...> System(const char* name = nullptr);

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
#include "Lumora/Aether/System.h"

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

	template <typename T>
	void World::RemoveResource()
	{
		LM_PROFILE_FUNCTION();

		m_World.remove<T>();
	}

	template <typename Phase>
	PhaseBuilder World::AddPhase()
	{
		LM_PROFILE_FUNCTION();

		return PhaseBuilder(*this, Entity(m_World.entity<Phase>()));
	}

	template <typename... Components>
	SystemBuilder<Components...> World::System(const char* name)
	{
		return SystemBuilder<Components...>(*this, name);
	}
}
