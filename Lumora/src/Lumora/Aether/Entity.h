#pragma once

#include "Lumora/Aether/Base.h"

namespace Lumora::Aether
{
	struct Entity
	{
		explicit Entity(const flecs::entity& e): m_Entity(e) {}
		explicit Entity(flecs::entity&& e)
		    : m_Entity(std::move(e)) {}
		
		flecs::entity& Raw() { return m_Entity; }
		flecs::entity Raw() const { return m_Entity; }
	private:
		flecs::entity m_Entity;
	};
}