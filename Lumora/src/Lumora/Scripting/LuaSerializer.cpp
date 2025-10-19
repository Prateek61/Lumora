#include "LMPCH.h"
#include "LuaSerializer.h"

namespace
{
	const Lumora::LuaSerializer::TypeInfo& GetTypeInfo(const std::string& typeName)
	{
		LM_PROFILE_FUNCTION();

		auto itr = Lumora::LuaSerializer::GetTypeRegistry().find(typeName);
		if ( itr == Lumora::LuaSerializer::GetTypeRegistry().end() )
		{
			LM_CORE_SERIALIZER_ERROR("Type not registered: {}", typeName);
			throw std::runtime_error("Type not registered: " + typeName);
		}

		return itr->second;
	}
}

namespace Lumora
{
	LuaSerializer::LuaSerializer()
	{
		LM_PROFILE_FUNCTION();

		m_Lua.open_libraries(sol::lib::base, sol::lib::table, sol::lib::string);
	}

	Ref<void> LuaSerializer::DeserializeFromFile(const std::string& typeName, const std::filesystem::path& file)
	{
		LM_PROFILE_FUNCTION();

		LM_CORE_SERIALIZER_TRACE("Deserializing from Lua file: {}", file.string());

		LM_LOCK_WRITE_AUTO();

		const auto& type_info = GetTypeInfo(typeName);

		// Load the script
		sol::load_result script = m_Lua.load_file(file.string());
		if ( !script.valid() )
		{
			sol::error err = script;
			LM_CORE_SERIALIZER_ERROR("Failed to load script from file '{}': {}", file.string(), err.what());
			return nullptr;
		}

		sol::protected_function_result result = script();
		if ( !result.valid() )
		{
			sol::error err = result;
			LM_CORE_SERIALIZER_ERROR("Failed to execute script from file '{}': {}", file.string(), err.what());
			return nullptr;
		}

		sol::object obj = result;

		return type_info.FromLuaFunction(obj);
	}

	Ref<void> LuaSerializer::DeserializeFromSolObject(const std::string& typeName, const sol::object& obj)
	{
		LM_PROFILE_FUNCTION();
		LM_LOCK_WRITE_AUTO();

		const auto& type_info = GetTypeInfo(typeName);
		return type_info.FromLuaFunction(obj);
	}

	Ref<void> LuaSerializer::DeserializeFromLuaScript(const std::string& typeName, const std::string& s)
	{
		LM_PROFILE_FUNCTION();
		LM_LOCK_WRITE_AUTO();

		const auto& type_info = GetTypeInfo(typeName);

		// Load the script
		sol::load_result script = m_Lua.load(s);
		if ( !script.valid() )
		{
			sol::error err = script;
			LM_CORE_SERIALIZER_ERROR("Failed to load script: {}", err.what());
			return nullptr;
		}

		sol::protected_function_result result = script();
		if ( !result.valid() )
		{
			sol::error err = result;
			LM_CORE_SERIALIZER_ERROR("Failed to execute script: {}", err.what());
			return nullptr;
		}

		sol::object obj = result;

		return type_info.FromLuaFunction(obj);
	}

	std::string LuaSerializer::SerializeToLuaScript(const std::string& typeName, const void* value, int indent)
	{
		LM_PROFILE_FUNCTION();
		LM_LOCK_WRITE_AUTO();

		const auto& type_info = GetTypeInfo(typeName);

		std::stringstream ss;
		type_info.ToLuaScriptFunction(value, ss, indent);
		return ss.str();
	}
}
