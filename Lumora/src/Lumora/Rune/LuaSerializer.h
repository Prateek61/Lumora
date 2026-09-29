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
		Result<T> DeserializeFromFile(const std::filesystem::path& file);
		template <typename T>
		Result<T> DeserializeFromLuaScript(const std::string_view& script);

		// Serialize
		template <typename T>
		std::string SerializeToScript(const T& data);

		// Type Registration
		template <typename T>
		void RegisterType(const std::string& name);

		// Runtime Functions through name
		Result<Ref<void>> DeserializeFromFile(const std::string& typeName, const std::filesystem::path& file);
		Result<Ref<void>> DeserializeFromLuaScript(const std::string& typeName, const std::string_view& script);
		Result<std::string> SerializeToScript(const std::string& typeName, const void* data);

		WriteLock<RWMutex> LockLuaState();

	private:
		RWMutex m_TypeRegistryMutex;
		sol::state m_Lua;

		struct TypeInfo
		{
			std::string Name;
			std::function<std::string(const void* valuePtr)> ToLuaScriptFunc;
			std::function<Result<Ref<void>>(const std::filesystem::path& file)> FromFileFunc;
			std::function<Result<Ref<void>>(const std::string_view& script)> FromLuaScriptFunc;
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
	Result<T> LuaSerializer::DeserializeFromFile(const std::filesystem::path& file)
	{
		LM_PROFILE_FUNCTION();
		LM_CORE_TRACE("Deserializing type: {}, from file: {}", typeid(T).name(), file.string());

		auto lock = WriteLock(LuaStateMutex);

		sol::load_result script = m_Lua.load_file(file.string());
		if (!script.valid())
		{
			sol::error err = script;
			return MakeError(std::filesystem::exists(file) ? ErrorCode::Parse : ErrorCode::NotFound, err.what());
		}

		sol::protected_function_result result = script();
		if (!result.valid())
		{
			sol::error err = result;
			return MakeError(ErrorCode::Script, err.what());
		}

		return WithContext(Serialization::FromLua<T>(sol::object(result)), "loading '{}'", file.string());
	}

	template <typename T>
	Result<T> LuaSerializer::DeserializeFromLuaScript(const std::string_view& script)
	{
		LM_PROFILE_FUNCTION();
		LM_CORE_TRACE("Deserializing type: {}, from script: {}", typeid(T).name(), script);

		auto lock = WriteLock(LuaStateMutex);

		sol::load_result loaded_script = m_Lua.load(script);
		if (!loaded_script.valid())
		{
			sol::error err = loaded_script;
			return MakeError(ErrorCode::Parse, err.what());
		}

		sol::protected_function_result result = loaded_script();
		if (!result.valid())
		{
			sol::error err = result;
			return MakeError(ErrorCode::Script, err.what());
		}

		sol::object obj = result;
		return WithContext(Serialization::FromLua<T>(obj), "loading from script");
	}

	template <typename T>
	std::string LuaSerializer::SerializeToScript(const T& data)
	{
		LM_PROFILE_FUNCTION();
		LM_CORE_TRACE("Serializing type: {}, to script", typeid(T).name());

		return Serialization::ToLuaScript(data);
	}

	template <typename T>
	void LuaSerializer::RegisterType(const std::string& name)
	{
		LM_PROFILE_FUNCTION();

		auto lock = WriteLock(m_TypeRegistryMutex);

		std::function<std::string(const void* valuePtr)> to_lua_script_func = [this
			](const void* valuePtr) -> std::string
		{
			const T* typedPtr = static_cast<const T*>(valuePtr);
			return SerializeToScript(*typedPtr);
		};

		std::function<Result<Ref<void>>(const std::filesystem::path& file)> from_file_func = [this](const std::filesystem::path& file) -> Result<Ref<void>>
		{
			// DeserializeFromFile<T> already names the file in the error; repeating it here would double the context.
			auto result = DeserializeFromFile<T>(file);
			if (!result)
			{
				return std::unexpected(std::move(result.error()));
			}
			return StaticRefCast<void>(CreateRef<T>(std::move(result.value())));
		};

		std::function<Result<Ref<void>>(const std::string_view& script)> from_lua_script_func = [this](const std::string_view& script) -> Result<Ref<void>>
		{
			auto result = DeserializeFromLuaScript<T>(script);
			if (!result)
			{
				return std::unexpected(std::move(result.error()));
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
		if (!m_TypeRegistry.emplace(name, std::move(info)).second)
		{
			LM_CORE_WARN("LuaSerializer: type name '{}' is already registered; keeping the first registration", name);
		}
	}
}
