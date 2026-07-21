#pragma once

#include "Lumora/Rune/Serialization/Concepts.h"
#include "Lumora/Rune/Lua/LuaBase.h"
#include "Lumora/Rune/Reflect.h"

#include <charconv>
#include <cmath>
#include <filesystem>
#include <ostream>
#include <string_view>

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

	namespace Detail
	{
		/// Lua reserved words.
		inline bool IsReservedWord(std::string_view s)
		{
			constexpr std::string_view keywords[]{
				"and", "break", "do", "else", "elseif", "end", "false", "for", "function", "goto", "if",
				"in", "local", "nil", "not", "or", "repeat", "return", "then", "true", "until", "while"
			};

			for (const std::string_view kw : keywords)
			{
				if (kw == s) return true;
			}
			return false;
		}

		/// Can `s` be written as a bare `s = value` table key, or does it need `["s"] = value`?
		inline bool IsBareKey(std::string_view s)
		{
			constexpr auto is_alpha = [](char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_'; };
			constexpr auto is_alnum = [=](char c) { return is_alpha(c) || (c >= '0' && c <= '9'); };

			if (s.empty() || !is_alpha(s.front())) return false;
			for (const char c : s)
			{
				if (!is_alnum(c)) return false;
			}
			return !IsReservedWord(s);
		}

		inline void WriteQuotedString(std::string_view value, std::ostream& stream)
		{
			stream << '"';
			for (const unsigned char c : value)
			{
				switch (c)
				{
				case '\\': stream << "\\\\"; break;
				case '"': stream << "\\\""; break;
				case '\n': stream << "\\n"; break;
				case '\r': stream << "\\r"; break;
				case '\t': stream << "\\t"; break;
				default:
					if (c < 0x20 || c == 0x7F)
					{
						// Padded to 3 digits so a following digit can't get swallowed into the escape
						stream << '\\' << static_cast<char>('0' + c / 100)
							<< static_cast<char>('0' + c / 10 % 10)
							<< static_cast<char>('0' + c % 10);
					}
					else
					{
						// >= 0x80 passes straight through; Lua strings are byte strings, so UTF-8 survives
						stream << c;
					}
					break;
				}
			}
			stream << '"';
		}

		/// `name = ` where Lua accepts it bare, `["name"] = ` where it doesn't.
		inline void WriteNamedKey(std::string_view name, std::ostream& stream)
		{
			if (IsBareKey(name))
			{
				stream << name << " = ";
				return;
			}

			stream << '[';
			WriteQuotedString(name, stream);
			stream << "] = ";
		}

		template <Arithmetic T>
		void WriteNumber(const T& value, std::ostream& stream)
		{
			if constexpr (std::is_floating_point_v<T>)
			{
				// Lua has no literal for either. Arithmetic, not math.huge: the script must load in a
				// state with no math library (LuaSerializer opens only base/string/table).
				if (std::isnan(value))
				{
					stream << "(0/0)";
					return;
				}
				if (std::isinf(value))
				{
					stream << (value < 0 ? "(-1/0)" : "(1/0)");
					return;
				}

				// Shortest form that reads back bit-identical
				char buf[40];
				const auto [end, ec] = std::to_chars(buf, buf + sizeof(buf), value);
				if (ec != std::errc{})
				{
					stream << value;
					return;
				}

				const std::string_view text(buf, static_cast<size_t>(end - buf));
				stream << text;

				// Keep Lua 5.4's float subtype: bare `3` comes back an integer, `3.0` stays a float
				if (text.find_first_of(".eE") == std::string_view::npos)
				{
					stream << ".0";
				}
			}
			else if constexpr (sizeof(T) == 1)
			{
				// 1-byte types are numbers, not characters
				stream << static_cast<int>(value);
			}
			else
			{
				stream << value;
			}
		}
	}

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

		/// Numeric keys must be bracketed — `1 = v` is a syntax error, `[1] = v` is the table constructor form.
		template <Arithmetic K>
		void WriteKey(const K& key)
		{
			Stream << '[';
			Detail::WriteNumber(key, Stream);
			Stream << "] = ";
		}

		template <StringLike K>
		void WriteKey(const K& key)
		{
			Detail::WriteNamedKey(key, Stream);
		}

		void WriteFieldName(std::string_view name)
		{
			Detail::WriteNamedKey(name, Stream);
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
		Detail::WriteNumber(value, backend.Stream);
	}

	template <>
	inline void ToLuaString<bool>(const bool& value, LuaScriptBackend& backend)
	{
		backend.Stream << (value ? "true" : "false");
	}

	template <>
	inline void ToLuaString<std::string>(const std::string& value, LuaScriptBackend& backend)
	{
		Detail::WriteQuotedString(value, backend.Stream);
	}

	template <>
	inline void ToLuaString<std::filesystem::path>(const std::filesystem::path& value, LuaScriptBackend& backend)
	{
		Detail::WriteQuotedString(value.generic_string(), backend.Stream);
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
			backend.WriteFieldName(name);
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
