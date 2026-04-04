#pragma once

#include "Lumora/Aether/Base.h"

namespace Lumora::Aether
{
	class System
	{
	public:
		explicit System(const flecs::system& s): m_System(s) {}
		
		flecs::system& Raw() { return m_System; }
		flecs::system Raw() const { return m_System; }
	private:
		flecs::system m_System;
	};
}