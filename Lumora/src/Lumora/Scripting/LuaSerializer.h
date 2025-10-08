#pragma once

#include "Lumora/Common/Threading.h"
#include "Lumora/Scripting/Lua/LuaBase.h"
#include "Lumora/Scripting/Lua/Serialize.h"

#include <unordered_map>

namespace Lumora
{
	class LuaSerializer
	{
		// Class that wraps sol::state
		// Provides an interface to serialize and deserialize structs to and from lua
	public:
		// Public for external locking
		LM_MUTEX_AUTO();

		LuaSerializer();

		// Not Thread Safe
		sol::state& GetLuaState() { return m_Lua; }

		// Deserialize
		template <typename T>
		T DeserializeFromFile(const std::filesystem::path& file);
		template <typename T>
		T DeserializeFromSolObject(const sol::object& obj);
		template <typename T>
		T DeserializeFromLuaScript(const std::string& script);

		// Serialize
		template <typename T>
		void SerializeToFile(const T& value, const std::filesystem::path& file);
		template <typename T>
		sol::object SerializeToSolObject(const T& value);
		template <typename T>
		std::string SerializeToLuaScript(const T& value, int indent = 2);

		// Non Templated Variants
		Ref<void> DeserializeFromFile(const std::string& typeName, const std::filesystem::path& file);
		Ref<void> DeserializeFromSolObject(const std::string& typeName, const sol::object& obj);
		Ref<void> DeserializeFromLuaScript(const std::string& typeName, const std::string& s);
		void SerializeToFile(const std::string& typeName, const void* value, const std::filesystem::path& file);
		std::string SerializeToLuaScript(const std::string& typeName, const void* value, int indent = 2);

	private:
		sol::state m_Lua;

	public:
		// Type Registry
		struct TypeInfo
		{
			std::string name;
			std::function<void(const void* valuePtr, std::ostream& os, int indent)> ToLuaScriptFunction;
			std::function<Ref<void>(const sol::object& obj)> FromLuaFunction;
			size_t size;
		};

		using Registry = std::unordered_map<std::string, TypeInfo>;

		static Registry& GetTypeRegistry()
		{
			static Registry registry;
			return registry;
		}
	};

	namespace Serialize
	{
		struct LuaTypeRegistrar
		{
			LuaTypeRegistrar(const char* name, LuaSerializer::TypeInfo info)
			{
				LuaSerializer::GetTypeRegistry()[name] = std::move(info);
			}

			LuaTypeRegistrar(const std::string& name, LuaSerializer::TypeInfo info)
			{
				LuaSerializer::GetTypeRegistry()[name] = std::move(info);
			}
		};
	}
}

// Template Implementations
namespace Lumora
{
	template <typename T>
	T LuaSerializer::DeserializeFromFile(const std::filesystem::path& file)
	{
		LM_PROFILE_FUNCTION();
		LM_LOCK_WRITE_AUTO();

		sol::load_result script = m_Lua.load_file(file.string());
		if ( !script.valid() )
		{
			sol::error sol_err = script;
			throw Lua::LuaError("Failed to load script: " + std::string(sol_err.what()));
		}

		sol::protected_function_result result = script();
		if ( !result.valid() )
		{
			sol::error sol_err = result;
			throw Lua::LuaError("Failed to execute script: " + std::string(sol_err.what()));
		}

		sol::object obj = result;
		return Serialize::FromLua<T>(obj);
	}

	template <typename T>
	T LuaSerializer::DeserializeFromSolObject(const sol::object& obj)
	{
		LM_PROFILE_FUNCTION();
		LM_LOCK_WRITE_AUTO();

		auto tab = obj.as<sol::table>();
		return Serialize::FromLua<T>(tab);
	}

	template <typename T>
	T LuaSerializer::DeserializeFromLuaScript(const std::string& script)
	{
		LM_PROFILE_FUNCTION();
		LM_LOCK_WRITE_AUTO();

		sol::load_result loadedScript = m_Lua.load(script);
		if ( !loadedScript.valid() )
		{
			sol::error sol_err = loadedScript;
			throw Lua::LuaError("Failed to load script: " + std::string(sol_err.what()));
		}

		sol::protected_function_result result = loadedScript();
		if ( !result.valid() )
		{
			sol::error sol_err = result;
			throw Lua::LuaError("Failed to execute script: " + std::string(sol_err.what()));
		}

		sol::object obj = result;
		return Serialize::FromLua<T>(obj);
	}

	template <typename T>
	void LuaSerializer::SerializeToFile(const T& value, const std::filesystem::path& file)
	{
		LM_PROFILE_FUNCTION();
		LM_LOCK_WRITE_AUTO();

		static_assert("Not Implemented");
	}

	template <typename T>
	sol::object LuaSerializer::SerializeToSolObject(const T& value)
	{
		LM_PROFILE_FUNCTION();
		LM_LOCK_WRITE_AUTO();

		return Serialize::ToLua<T>(m_Lua, value);
	}

	template <typename T>
	std::string LuaSerializer::SerializeToLuaScript(const T& value, int indent)
	{
		LM_PROFILE_FUNCTION();
		LM_LOCK_WRITE_AUTO();

		std::stringstream ss;
		Serialize::ToLuaScript<T>(value, ss, indent);
		return ss.str();
	}
}

// Macro to register a type
#define LM_REGISTER_FOR_SERIALIZATION_NAMED(TYPE, NAME)								 \
	namespace {																		 \
		Lumora::Serialize::LuaTypeRegistrar _auto_register_##TYPE(				     \
			NAME,																     \
			{																	     \
				NAME,															     \
				Lumora::Serialize::GetToLuaScriptFunction<TYPE>(),				     \
				Lumora::Serialize::GetFromLuaFunction<TYPE>(),					     \
				sizeof(TYPE)													     \
			}																	     \
		);																		     \
	}

#define LM_REGISTER_FOR_SERIALIZATION_NAMED_VAR(TYPE, NAME)			                 \
	Lumora::Serialize::LuaTypeRegistrar _auto_register_##TYPE(                       \
		NAME,                                                                        \
		{                                                                            \
			NAME,                                                                    \
			Lumora::Serialize::GetToLuaScriptFunction<TYPE>(),                       \
			Lumora::Serialize::GetFromLuaFunction<TYPE>(),                           \
			sizeof(TYPE)                                                             \
		});

#define LM_REGISTER_FOR_SERIALIZATION(TYPE) LM_REGISTER_FOR_SERIALIZATION_NAMED(TYPE, #TYPE)
