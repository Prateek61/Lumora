#include "LMPCH.h"
#include "LuaSerializer.h"

namespace Lumora::Rune
{
	LuaSerializer::LuaSerializer()
	{
		LM_PROFILE_FUNCTION();

		m_Lua.open_libraries(sol::lib::base, sol::lib::string, sol::lib::table);
	}

	Ref<void> LuaSerializer::DeserializeFromFile(const std::string& typeName, const std::filesystem::path& file)
	{
		LM_PROFILE_FUNCTION();

		auto type_info_opt = GetTypeInfo(typeName);
		if (!type_info_opt)
		{
			LM_CORE_ERROR("Type '{}' is not registered for deserialization", typeName);
			return nullptr;
		}

		auto& type_info = type_info_opt.value();

		if (!type_info.FromFileFunc)
		{
			LM_CORE_ERROR("Type '{}' does not have a FromFile deserialization function", typeName);
			return nullptr;
		}

		return type_info.FromFileFunc(file);
	}

	Ref<void> LuaSerializer::DeserializeFromLuaScript(const std::string& typeName, const std::string_view& script)
	{
		LM_PROFILE_FUNCTION();
		
		auto type_info_opt = GetTypeInfo(typeName);
		if (!type_info_opt)
		{
			LM_CORE_ERROR("Type '{}' is not registered for deserialization", typeName);
			return nullptr;
		}

		auto& type_info = type_info_opt.value();

		if (!type_info.FromLuaScriptFunc)
		{
			LM_CORE_ERROR("Type '{}' does not have a FromLuaScript deserialization function", typeName);
			return nullptr;
		}

		return type_info.FromLuaScriptFunc(script);
	}

	std::optional<std::string> LuaSerializer::SerializeToScript(const std::string& typeName, const void* data)
	{
		LM_PROFILE_FUNCTION();

		auto type_info_opt = GetTypeInfo(typeName);
		if (!type_info_opt)
		{
			LM_CORE_ERROR("Type '{}' is not registered for serialization", typeName);
			return std::nullopt;
		}

		auto& type_info = type_info_opt.value();

		if (!type_info.ToLuaScriptFunc)
		{
			LM_CORE_ERROR("Type '{}' does not have a ToLuaScript serialization function", typeName);
			return std::nullopt;
		}

		return type_info.ToLuaScriptFunc(data);
	}

	std::optional<LuaSerializer::TypeInfo> LuaSerializer::GetTypeInfo(const std::string& typeName)
	{
		LM_PROFILE_FUNCTION();

		auto lock = ReadLock(m_TypeRegistryMutex);

		auto itr = m_TypeRegistry.find(typeName);
		if (itr != m_TypeRegistry.end())
		{
			return itr->second;
		}
		else
		{
			return std::nullopt;
		}
	}


}
