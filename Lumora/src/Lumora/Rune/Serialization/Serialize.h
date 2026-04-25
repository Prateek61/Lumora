#pragma once

#include "Lumora/Rune/Serialization/Concepts.h"
#include "Lumora/Rune/Lua/LuaBase.h"
#include "Lumora/Rune/Serialization/Reflect.h"

namespace Lumora::Rune::Serialization
{
	//////////////////////////////////////////////////////////////////////////////////
	/// Lua To C++ Deserialization
	//////////////////////////////////////////////////////////////////////////////////


	template <typename T>
	T FromLua(const sol::object& obj)
	{
		static_assert(sizeof(T) == 0, "FromLua is not implemented for this type.");
		return T{};
	}

	template <SolSupportedType T>
	T FromLua(const sol::object& obj)
	{
		if (!obj.is<T>())
		{
			throw Lua::LuaError(
				"Type mismatch: Expected " + std::string(typeid(T).name()) + ", got " + sol::type_name(obj.lua_state(), obj.get_type()));
		}

		return obj.as<T>();
	}

	template <>
	inline std::filesystem::path FromLua<std::filesystem::path>(const sol::object& obj)
	{
		return {FromLua<std::string>(obj)};
	}

	template <Reflect::Reflectable T>
	T FromLua(const sol::object& obj)
	{
		if (!obj.is<sol::table>())
		{
			throw Lua::LuaError("Type mismatch: Expected table for reflectable type, got " +
			                    sol::type_name(obj.lua_state(), obj.get_type()));
		}
		auto tbl = obj.as<sol::table>();

		T t{};
		Reflect::ForEach(t, [&](const char* name, auto& field)
		{
			using FieldType = std::decay_t<decltype(field)>;
			if (tbl[name].valid())
			{
				try
				{
					field = FromLua<FieldType>(tbl[name]);
				}
				catch (const Lua::LuaError& e)
				{
					LM_CORE_ERROR("Error deserializing field '{}': {}", name, e.what());
					throw;
				}
			}
		});

		return t;
	}

	//////////////////////////////////////////////////////////////////////////////////
	/// C++ to Lua Script Serialization
	//////////////////////////////////////////////////////////////////////////////////

	struct LuaScriptBackend
	{
		LuaScriptBackend(std::ostream& stream) : Stream(stream) {}
		LuaScriptBackend(const LuaScriptBackend& other) = delete;
		~LuaScriptBackend() = default;

		void Indent()
		{
			CurrentIndentation += IndentationSize;
		}

		void Dedent()
		{
			CurrentIndentation -= (CurrentIndentation >= IndentationSize) ? IndentationSize : CurrentIndentation;
		}

		void WriteIndentation()
		{
			Stream << std::string(CurrentIndentation, ' ');
		}

		void Scope()
		{
			Stream << "{\n";
			Indent();
		}

		void EndScope()
		{
			Dedent();
			WriteIndentation();
			Stream << "}";
		}

		template <KeyType K>
		void WriteKey(const K& key)
		{
			Stream << key << " = ";
		}

		uint32_t CurrentIndentation = 0;
		uint32_t IndentationSize = 4;
		std::ostream& Stream;
	};

	template <typename T>
	void ToLuaString(const T& value, LuaScriptBackend& backend)
	{
		static_assert(sizeof(T) == 0, "ToLuaString is not implemented for this type.");
	}

	// Scalar Types
	template <Arithmetic T>
	void ToLuaString(const T& value, LuaScriptBackend& backend)
	{
		backend.Stream << value;
	}

	template <>
	inline void ToLuaString<bool>(const bool& value, LuaScriptBackend& backend)
	{
		backend.Stream << (value ? "true" : "false");
	}

	template <>
	inline void ToLuaString<std::string>(const std::string& value, LuaScriptBackend& backend)
	{
		backend.Stream << "\"" << value << "\"";
	}

	template <VectorLike V>
	void ToLuaString(const V& vec, LuaScriptBackend& backend)
	{
		using ValueType = typename V::value_type;
		backend.Scope();
		for (const auto& item : vec)
		{
			backend.WriteIndentation();
			ToLuaString(item, backend);
			backend.Stream << ",\n";
		}
		backend.EndScope();
	}

	template <MapLike M>
	void ToLuaString(const M& map, LuaScriptBackend& backend)
	{
		using KeyType = typename M::key_type;
		using ValueType = typename M::mapped_type;
		backend.Scope();
		for (const auto& [key, value] : map)
		{
			backend.WriteIndentation();
			backend.WriteKey(key);
			ToLuaString(value, backend);
			backend.Stream << ",\n";
		}
		backend.EndScope();
	}

	template <Reflect::Reflectable T>
	void ToLuaString(const T& obj, LuaScriptBackend& backend)
	{
		backend.Scope();
		Reflect::ForEach(obj, [&](const char* name, const auto& field)
		{
			backend.WriteIndentation();
			backend.Stream << name << " = ";
			ToLuaString(field, backend);
			backend.Stream << ",\n";
		});
		backend.EndScope();
	}

	template <typename T>
	void ToLuaScript(const T& obj, std::ostream& stream)
	{
		LuaScriptBackend backend(stream);
		backend.Stream << "return ";
		ToLuaString(obj, backend);
	}

	template <typename T>
	std::string ToLuaScript(const T& obj)
	{
		std::stringstream ss;
		ToLuaScript(obj, ss);
		return ss.str();
	}
}
