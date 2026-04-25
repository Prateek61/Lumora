#pragma once

#include "Lumora/Core/Threading.h"
#include "Lumora/Rune/Serialization/Serialize.h"
#include "Lumora/Rune/Lua/LuaBase.h"

#include <unordered_map>
#include <optional>

namespace Lumora::Rune
{
	class LuaSerializer
	{
	public:
		RWMutex LuaStateMutex;

		LuaSerializer();

		// Not Thread Safe, Get the lock before calling and using the returned state
		sol::state& GetLuaState() { return m_Lua; }

		// Deserialize
		template <typename T>
		std::optional<T> DeserializeFromFile(const std::filesystem::path& file);
		template <typename T>
		std::optional<T> DeserializeFromLuaScript(const std::string_view& script);

		// Serialize
		template <typename T>
		std::optional<std::string> SerializeToScript(const T& data);

		// Type Registration
		template <typename T>
		void RegisterType(const std::string& name);

		// Runtime Functions through name
		Ref<void> DeserializeFromFile(const std::string& typeName, const std::filesystem::path& file);
		Ref<void> DeserializeFromLuaScript(const std::string& typeName, const std::string_view& script);
		std::optional<std::string> SerializeToScript(const std::string& typeName, const void* data);

	private:
		RWMutex m_TypeRegistryMutex;
		sol::state m_Lua;

		struct TypeInfo
		{
			std::string Name;
			std::function<std::optional<std::string>(const void* valuePtr)> ToLuaScriptFunc;
			std::function<Ref<void>(const std::filesystem::path& file)> FromFileFunc;
			std::function<Ref<void>(const std::string_view& script)> FromLuaScriptFunc;
			size_t Size;
		};

		using Registry = std::unordered_map<std::string, TypeInfo>;

		Registry m_TypeRegistry;

	private:
		std::optional<TypeInfo> GetTypeInfo(const std::string& typeName);
	};
}

// Template Implementations
namespace Lumora::Rune
{
	template <typename T>
	std::optional<T> LuaSerializer::DeserializeFromFile(const std::filesystem::path& file)
	{
		LM_PROFILE_FUNCTION();

		LM_CORE_TRACE("Deserializing type: {}, from file: {}", typeid(T).name(), file.string());

		auto lock = WriteLock(LuaStateMutex);

		sol::load_result script = m_Lua.load_file(file.string());
		if (!script.valid())
		{
			sol::error err = script;
			LM_CORE_ERROR("Failed to load script from file: {}:\n{}", file.string(), err.what());
			return std::nullopt;
		}

		sol::protected_function_result result = script();
		if (!result.valid())
		{
			sol::error err = result;
			LM_CORE_ERROR("Failed to execute script from file: {}:\n{}", file.string(), err.what());
			return std::nullopt;
		}

		sol::object obj = result;
		try
		{
			return Serialization::FromLua<T>(obj);
		}
		catch (const Lua::LuaError& e)
		{
			LM_CORE_ERROR("Error Deserializing type({}) from file({}):\n{}", typeid(T).name(), file.string(), e.what());
			return std::nullopt;
		}
	}

	template <typename T>
	std::optional<T> LuaSerializer::DeserializeFromLuaScript(const std::string_view& script)
	{
		LM_PROFILE_FUNCTION();

		LM_CORE_TRACE("Deserializing type: {}, from script: {}", typeid(T).name(), script);

		auto lock = WriteLock(LuaStateMutex);

		sol::load_result loaded_script = m_Lua.load(script);
		if (!loaded_script.valid())
		{
			sol::error err = loaded_script;
			LM_CORE_ERROR("Failed to load script:\n{}:\n{}", script, err.what());
			return std::nullopt;
		}

		sol::protected_function_result result = loaded_script();
		if (!result.valid())
		{
			sol::error err = result;
			LM_CORE_ERROR("Failed to execute script:\n{}:\n{}", script, err.what());
			return std::nullopt;
		}

		sol::object obj = result;
		try
		{
			return Serialization::FromLua<T>(obj);
		}
		catch (const Lua::LuaError& e)
		{
			LM_CORE_ERROR("Error Deserializing type({}) from script:\n{}", typeid(T).name(), e.what());
			return std::nullopt;
		}
	}

	template <typename T>
	std::optional<std::string> LuaSerializer::SerializeToScript(const T& data)
	{
		LM_PROFILE_FUNCTION();

		LM_CORE_TRACE("Serializing type: {}, to script", typeid(T).name());

		try
		{
			return Serialization::ToLuaScript(data);
		}
		catch (const Lua::LuaError& e)
		{
			LM_CORE_ERROR("Error Serializing type({}) to script:\n{}", typeid(T).name(), e.what());
			return std::nullopt;
		}
	}

	template <typename T>
	void LuaSerializer::RegisterType(const std::string& name)
	{
		LM_PROFILE_FUNCTION();

		auto lock = WriteLock(m_TypeRegistryMutex);

		std::function<std::optional<std::string>(const void* valuePtr)> to_lua_script_func = [this
			](const void* valuePtr) -> std::optional<std::string>
		{
			const T* typedPtr = static_cast<const T*>(valuePtr);
			return SerializeToScript(*typedPtr);
		};

		std::function<Ref<void>(const std::filesystem::path& file)> from_file_func = [this](const std::filesystem::path& file) -> Ref<void>
		{
			auto result = DeserializeFromFile<T>(file);
			if (!result.has_value())
			{
				return nullptr;
			}
			return StaticRefCast<void>(CreateRef<T>(std::move(result.value())));
		};

		std::function<Ref<void>(const std::string_view& script)> from_lua_script_func = [this](const std::string_view& script) -> Ref<void>
		{
			auto result = DeserializeFromLuaScript<T>(script);
			if (!result.has_value())
			{
				return nullptr;
			}
			return StaticRefCast<void>(CreateRef<T>(std::move(result.value())));
		};

		// Register the type info
		TypeInfo info{
			.Name = name,
			.ToLuaScriptFunc = to_lua_script_func,
			.FromFileFunc = from_file_func,
			.FromLuaScriptFunc = from_lua_script_func,
			.Size = sizeof(T)
		};
		m_TypeRegistry.emplace(name, std::move(info));
	}
}
