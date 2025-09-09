#include "LMPCH.h"
#include "LuaSerializer.h"

namespace
{
	const Lumora::LuaSerializer::TypeInfo& GetTypeInfo(const std::string& typeName)
	{
		auto itr = Lumora::LuaSerializer::GetTypeRegistry().find(typeName);
		if ( itr == Lumora::LuaSerializer::GetTypeRegistry().end() )
		{
			throw std::runtime_error("Type not registered: " + typeName);
		}

		return itr->second;
	}
}

namespace Lumora
{
	LuaSerializer::LuaSerializer()
	{
		m_Lua.open_libraries(sol::lib::base, sol::lib::table, sol::lib::string);
	}

	Ref<void> LuaSerializer::DeserializeFromFile(const std::string& typeName, const std::filesystem::path& file)
	{
		LM_LOCK_WRITE_AUTO()

		const auto& type_info = GetTypeInfo(typeName);

		// Load the script
		sol::load_result script = m_Lua.load_file(file.string());
		if ( !script.valid() )
		{
			throw std::runtime_error("Failed to load script: " + file.string());
		}

		sol::protected_function_result result = script();
		if ( !result.valid() )
		{
			throw std::runtime_error("Failed to execute script: " + file.string());
		}

		sol::table tab = result;

		return type_info.FromLuaFunction(tab);
	}

	Ref<void> LuaSerializer::DeserializeFromSolObject(const std::string& typeName, const sol::object& obj)
	{
		LM_LOCK_WRITE_AUTO()

		const auto& type_info = GetTypeInfo(typeName);
		return type_info.FromLuaFunction(obj);
	}

	Ref<void> LuaSerializer::DeserializeFromLuaScript(const std::string& typeName, const std::string& s)
	{
		LM_LOCK_WRITE_AUTO()

		const auto& type_info = GetTypeInfo(typeName);

		// Load the script
		sol::load_result script = m_Lua.load(s);
		if ( !script.valid() )
		{
			throw std::runtime_error("Failed to load script: ");
		}

		sol::protected_function_result result = script();
		if ( !result.valid() )
		{
			throw std::runtime_error("Failed to execute script: ");
		}

		sol::table tab = result;

		return type_info.FromLuaFunction(tab);
	}

	std::string LuaSerializer::SerializeToLuaScript(const std::string& typeName, const void* value, int indent)
	{
		LM_LOCK_WRITE_AUTO()

		const auto& type_info = GetTypeInfo(typeName);

		std::stringstream ss;
		type_info.ToLuaScriptFunction(value, ss, indent);
		return ss.str();
	}
}
