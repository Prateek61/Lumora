#pragma once

#include "Lumora/Core/Base.h"
#include "sol/sol.hpp"


namespace Lumora::Rune
{
	namespace Lua
	{
		class LuaError : public std::runtime_error
		{
		public:
			LuaError(const std::string& message) : std::runtime_error(message) {}
			LuaError(const char* message) : std::runtime_error(message) {}
		};
	}
}