#pragma once

#include "Lumora/Aether/Base.h"

namespace Lumora::Aether
{
	template <typename... Components>
	class Query
	{
	public:
		explicit Query(const flecs::query<Components...>& q): m_Query(q) {}
		
		flecs::query<Components...>& Raw() { return m_Query; }
		flecs::query<Components...> Raw() const { return m_Query; }
	private:
		flecs::query<Components...> m_Query;
	};
}