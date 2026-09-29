#include "LMPCH.h"
#include "LuaSerializer.h"

namespace Lumora::Rune
{
	LuaSerializer::LuaSerializer()
	{
		LM_PROFILE_FUNCTION();

		m_Lua.open_libraries(sol::lib::base, sol::lib::string, sol::lib::table);
	}

	Result<Ref<void>> LuaSerializer::DeserializeFromFile(const std::string& typeName, const std::filesystem::path& file)
	{
		LM_PROFILE_FUNCTION();

		auto type_info_opt = GetTypeInfo(typeName);
		if (!type_info_opt)
		{
			return MakeError(ErrorCode::NotRegistered, fmt::format("Type '{}' is not registered", typeName));
		}

		auto& type_info = type_info_opt.value();
		if (!type_info.FromFileFunc)
		{
			return MakeError(ErrorCode::Invalid, fmt::format("Type '{}' has no FromFile function", typeName));
		}

		return type_info.FromFileFunc(file);
	}

	Result<Ref<void>> LuaSerializer::DeserializeFromLuaScript(const std::string& typeName, const std::string_view& script)
	{
		LM_PROFILE_FUNCTION();
		
		auto type_info_opt = GetTypeInfo(typeName);
		if (!type_info_opt)
		{
			return MakeError(ErrorCode::NotRegistered, fmt::format("Type '{}' is not registered", typeName));
		}

		auto& type_info = type_info_opt.value();

		if (!type_info.FromLuaScriptFunc)
		{
			return MakeError(ErrorCode::Invalid, fmt::format("Type '{}' has no FromLuaScript function", typeName));
		}

		return type_info.FromLuaScriptFunc(script);
	}

	Result<std::string> LuaSerializer::SerializeToScript(const std::string& typeName, const void* data)
	{
		LM_PROFILE_FUNCTION();

		auto type_info_opt = GetTypeInfo(typeName);
		if (!type_info_opt)
		{
			return MakeError(ErrorCode::NotRegistered, fmt::format("Type '{}' is not registered", typeName));
		}

		auto& type_info = type_info_opt.value();

		if (!type_info.ToLuaScriptFunc)
		{
			return MakeError(ErrorCode::Invalid, fmt::format("Type '{}' has no ToLuaScript function", typeName));
		}

		return type_info.ToLuaScriptFunc(data);
	}

	WriteLock<RWMutex> LuaSerializer::LockLuaState()
	{
		return WriteLock(LuaStateMutex);
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
