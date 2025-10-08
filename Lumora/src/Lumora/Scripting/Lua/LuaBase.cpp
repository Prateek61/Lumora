#include "LMPCH.h"

#include "LuaBase.h"

namespace Lumora
{
	sol::protected_function_result SolUtils::FileScriptResults(const std::filesystem::path& file, sol::state& m_Lua)
	{
		LM_PROFILE_FUNCTION();

		sol::load_result script = m_Lua.load_file(file.string());
		if (!script.valid())
		{
			sol::error err = script;
			throw Lua::LuaError("Failed to load script: " + std::string(err.what()));
		}

		sol::protected_function_result result = script();
		if (!result.valid())
		{
			sol::error err = result;
			throw Lua::LuaError("Failed to execute script: " + std::string(err.what()));
		}

		return result;
	}

	sol::protected_function_result SolUtils::ScriptResults(const std::string& script, sol::state& m_Lua)
	{
		LM_PROFILE_FUNCTION();

		sol::load_result loaded_script = m_Lua.load(script);
		if (!loaded_script.valid())
		{
			sol::error err = loaded_script;
			throw Lua::LuaError("Failed to load script: " + std::string(err.what()));
		}

		sol::protected_function_result result = loaded_script();
		if (!result.valid())
		{
			sol::error err = result;
			throw Lua::LuaError("Failed to execute script: " + std::string(err.what()));
		}

		return result;
	}
}