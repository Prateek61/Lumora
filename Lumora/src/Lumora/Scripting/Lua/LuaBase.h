#pragma once

#include "Lumora/Core/Base.h"
#include "sol/sol.hpp"
#include <filesystem>

#include <exception>

namespace Lumora
{
	namespace Lua
	{
		class LuaError : public std::runtime_error
		{
		public:
			LuaError(const std::string& message)
				: runtime_error(message)
			{
			}

			LuaError(const char* message)
				: runtime_error(message)
			{
			}
		};
	}

	class SolUtils
	{
	public:
		static sol::protected_function_result FileScriptResults(const std::filesystem::path& file, sol::state& m_Lua);
		static sol::protected_function_result ScriptResults(const std::string& script, sol::state& m_Lua);

	};
}