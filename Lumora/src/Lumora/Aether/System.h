#pragma once

#include "Lumora/Aether/Base.h"
#include "Lumora/Utilities/TimeStep.h"

namespace Lumora::Aether
{
	namespace Phases
	{
		struct None;
	}

	class System
	{
	public:
		explicit System(flecs::system&& s) : m_System(std::move(s)) {}
		System() = default;

		void Run(TimeStep deltaTime, void* param = nullptr) const;
		void Enable() const;
		void Disable() const;
		bool IsEnabled() const;

		flecs::system& Raw() { return m_System; }
		flecs::system Raw() const { return m_System; }

	private:
		flecs::system m_System;
	};

	template <typename... Components>
	class SystemBuilder
	{
	public:
		explicit SystemBuilder(World& world, const char* name = nullptr);

		/// Set the execution phase for this system.
		template <typename Phase>
		SystemBuilder& SetPhase();

		/// Indicates that the system will read from the specified component type.
		template <typename T>
		SystemBuilder& Read();
		/// Indicates that the system will write to the specified component type.
		template <typename T>
		SystemBuilder& Write();

		template <typename Func>
			requires std::is_invocable_v<Func, Components&...>
		System Each(Func&& func);
		template <typename Func>
			requires std::is_invocable_v<Func, Entity, Components&...>
		System Each(Func&& func);
		/// `row` is the entity's slot in its archetype table. It restarts at 0 for every table
		/// the query matches.
		template <typename Func>
			requires std::is_invocable_v<Func, size_t, Components&...>
		System Each(Func&& func);
		template <typename Func>
			requires std::is_invocable_v<Func, size_t, Entity, Components&...>
		System Each(Func&& func);

		template <typename Func>
		    requires std::is_invocable_v<Func, QueryRes> || std::is_invocable_v<Func, QueryRes&>
		System Run(Func&& func);


		flecs::system_builder<Components...>& Raw() { return m_SystemBuilder; }
		flecs::system_builder<Components...> Raw() const { return m_SystemBuilder; }

	private:
		flecs::system_builder<Components...> m_SystemBuilder;
		World& m_World;
	};
}

#include "Lumora/Aether/World.h"
#include "Lumora/Aether/QueryRes.h"

// Template Implementation
namespace Lumora::Aether
{
	inline void System::Run(TimeStep deltaTime, void* param) const
	{
		m_System.run(deltaTime.GetSeconds(), param);
	}

	inline void System::Enable() const
	{
		auto& _ = m_System.enable();
	}

	inline void System::Disable() const
	{
		auto& _ = m_System.disable();
	}

	inline bool System::IsEnabled() const
	{
		return m_System.enabled();
	}

	template <typename... Components>
	SystemBuilder<Components...>::SystemBuilder(World& world, const char* name)
		: m_SystemBuilder(world.Raw().system<Components...>(name)), m_World(world) {}

	template <typename... Components>
	template <typename Phase>
	SystemBuilder<Components...>& SystemBuilder<Components...>::SetPhase()
	{
		if constexpr (std::is_same_v<Phase, Phases::None>)
		{
			m_SystemBuilder.kind(0);
		}
		else
		{
			auto phase_entity = m_World.Raw().entity<Phase>();
			LM_CORE_ASSERT(phase_entity.is_valid() && phase_entity.has(flecs::Phase), "Phase does not exist or is not a phase.");

			m_SystemBuilder.kind(phase_entity);
		}

		return *this;
	}

	template <typename... Components>
	template <typename T>
	SystemBuilder<Components...>& SystemBuilder<Components...>::Read()
	{
		this->m_SystemBuilder.template read<T>();
		return *this;
	}

	template <typename... Components>
	template <typename T>
	SystemBuilder<Components...>& SystemBuilder<Components...>::Write()
	{
		this->m_SystemBuilder.template write<T>();
		return *this;
	}

	template <typename... Components>
	template <typename Func>
		requires std::is_invocable_v<Func, Components&...>
	System SystemBuilder<Components...>::Each(Func&& func)
	{
		return System{
			m_SystemBuilder.each(std::forward<Func>(func))
		};
	}

	template <typename... Components>
	template <typename Func>
	    requires std::is_invocable_v<Func, QueryRes> || std::is_invocable_v<Func, QueryRes&>
	System SystemBuilder<Components...>::Run(Func&& func)
	{
		return System{m_SystemBuilder.run([fn = std::forward<Func>(func)](flecs::iter& iter)
		{
			QueryRes res{iter};
			fn(res);
		})};
	}

	template <typename... Components>
	template <typename Func>
		requires std::is_invocable_v<Func, Entity, Components&...>
	System SystemBuilder<Components...>::Each(Func&& func)
	{
		return System{m_SystemBuilder.each([fn = std::forward<Func>(func)](flecs::entity entity, Components&... components)
		{
			fn(Entity{entity}, components...);
		})};
	}

	template <typename... Components>
	template <typename Func>
		requires std::is_invocable_v<Func, size_t, Components&...>
	System SystemBuilder<Components...>::Each(Func&& func)
	{
		return System{m_SystemBuilder.each([fn = std::forward<Func>(func)](flecs::iter&, size_t row, Components&... components)
		{
			fn(row, components...);
		})};
	}

	template <typename... Components>
	template <typename Func>
		requires std::is_invocable_v<Func, size_t, Entity, Components&...>
	System SystemBuilder<Components...>::Each(Func&& func)
	{
		return System{m_SystemBuilder.each([fn = std::forward<Func>(func)](flecs::iter& it, size_t row, Components&... components)
		{
			fn(row, Entity{it.entity(row)}, components...);
		})};
	}
}
