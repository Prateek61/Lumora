#pragma once

#include "Lumora/Scripting/Lua/LuaBase.h"
#include "visit_struct/visit_struct_intrusive.hpp"

namespace Lumora::Serialize
{
	// Helper: check if type is specialization of a template
	template <typename T, template <typename...> class Template>
	struct is_specialization_of : std::false_type
	{
	};

	template <template <typename...> class Template, typename... Args>
	struct is_specialization_of<Template<Args...>, Template> : std::true_type
	{
	};

	template <typename T, template <typename...> class Template>
	inline constexpr bool is_specialization_of_v = is_specialization_of<T, Template>::value;

	// ============================================================
	// Lua to C++ deserialization
	// ============================================================

	// Forward Declaration
	template <typename T>
	T FromLua(const sol::object& obj)
	{
		// Static assert to provide better error message
		static_assert(false, "FromLua: Unsupported type");
		return T{};
	}

	// Scalar types
	template<typename T>
		requires std::is_arithmetic_v<T> && !std::is_same_v<T, bool>
	T FromLua(const sol::object& obj)
	{
		return obj.as<T>();
	}

	template <>
	inline std::string FromLua<std::string>(const sol::object& obj)
	{
		return obj.as<std::string>();
	}

	template <>
	inline std::filesystem::path FromLua<std::filesystem::path>(const sol::object& obj)
	{
		return {obj.as<std::string>()};
	}

	template <>
	inline bool FromLua<bool>(const sol::object& obj)
	{
		return obj.as<bool>();
	}

	// std::vector<T>
	template <typename Vec>
		requires is_specialization_of_v<Vec, std::vector>
	Vec FromLua(const sol::object& obj)
	{
		using T = typename Vec::value_type;
		auto tbl = obj.as<sol::table>();
		std::vector<T> vec;
		for ( auto& pair : tbl )
		{
			vec.push_back(FromLua<T>(pair.second));
		}
		return vec;
	}

	// std::map<K, T>
	template <typename Map>
		requires is_specialization_of_v<Map, std::map>
	Map FromLua(const sol::object& obj)
	{
		using K = typename Map::key_type;
		using T = typename Map::mapped_type;
		auto tbl = obj.as<sol::table>();
		std::map<K, T> map;
		for ( auto& pair : tbl )
		{
			map[pair.first.as<K>()] = FromLua<T>(pair.second);
		}
		return map;
	}

	// For structs registered with visit_struct
	template <typename T>
		requires visit_struct::traits::is_visitable<T>::value
	T FromLua(const sol::object& obj)
	{
		auto tbl = obj.as<sol::table>();
		T t{};
		visit_struct::for_each(t, [&](const char* name, auto& value)
		{
			if ( tbl[name].valid() )
			{
				using FieldType = std::decay_t<decltype(value)>;
				value = FromLua<FieldType>(tbl[name]);
			}
		});
		return t;
	}

	// ============================================================
	// C++ to Lua serialization
	// ============================================================

	// Forward Declaration
	template <typename T>
	sol::object ToLua(sol::state_view& lua, const T& value)
	{
		static_assert(false, "ToLua: Unsupported type");
		return {};
	}

	// Scalar types
	template<typename T>
	sol::object ToLua(sol::state_view& lua, const T& value)
		requires std::is_arithmetic_v<T> && !std::is_same_v<T, bool>
	{
		return make_object(lua, value);
	}

	template <>
	inline sol::object ToLua<std::string>(sol::state_view& lua, const std::string& value)
	{
		return make_object(lua, value);
	}

	template <>
	inline sol::object ToLua<std::filesystem::path>(sol::state_view& lua, const std::filesystem::path& value)
	{
		return make_object(lua, value.string());
	}

	template <>
	inline sol::object ToLua<bool>(sol::state_view& lua, const bool& value)
	{
		return make_object(lua, value);
	}

	// std::vector<T>
	template <typename Vec>
		requires is_specialization_of_v<Vec, std::vector>
	sol::object ToLua(sol::state_view& lua, const Vec& vec)
	{
		using T = typename Vec::value_type;
		sol::table tbl = lua.create_table();
		for ( size_t i = 0; i < vec.size(); ++i )
		{
			tbl[i + 1] = ToLua<T>(lua, vec[i]);
		}
		return tbl;
	}

	// std::map<K, T>
	template <typename T>
		requires is_specialization_of_v<T, std::map>
	sol::object ToLua(sol::state_view& lua, const std::map<std::string, T>& map)
	{
		sol::table tbl = lua.create_table();
		for ( const auto& [k, v] : map )
		{
			tbl[k] = ToLua<T>(lua, v);
		}
		return tbl;
	}

	// For structs registered with visit_struct
	template <typename T>
		requires visit_struct::traits::is_visitable<T>::value
	sol::object ToLua(sol::state_view& lua, const T& t)
	{
		sol::table tbl = lua.create_table();
		visit_struct::for_each(t, [&](const char* name, const auto& value)
		{
			using FieldType = std::decay_t<decltype(value)>;
			tbl[name] = ToLua<FieldType>(lua, value);
		});
		return tbl;
	}

	// ============================================================
	// C++ to Lua String serialization
	// ============================================================

	// Forward Declaration
	template <typename T>
	void ToLuaString(const T& value, std::ostream& os, int curr_indent = 0, int indent = 2, bool key = false)
	{
		static_assert("ToLuaString: Unsupported type");
	}

	// Scalar Types
	template<typename T>
		requires std::is_arithmetic_v<T> && !std::is_same_v<T, bool>
	void ToLuaString(const T& value, std::ostream& os, int curr_indent, int indent, bool key)
	{
		os << value;
	}

	template<>
	inline void ToLuaString<bool>(const bool& value, std::ostream& os, int curr_indent, int indent, bool key)
	{
		os << (value ? "true" : "false");
	}

	template <>
	inline void ToLuaString<std::string>(const std::string& value, std::ostream& os, int curr_indent, int indent,
	                                     bool key)
	{
		os << "\"" << value << "\"";
	}

	template <>
	inline void ToLuaString<std::filesystem::path>(const std::filesystem::path& value, std::ostream& os,
	                                               int curr_indent, int indent, bool key)
	{
		if ( !key ) os << "\"" << value.string() << "\"";
		else os << value.string();
	}

	// std::vector<T>
	template <typename Vec>
		requires is_specialization_of_v<Vec, std::vector>
	void ToLuaString(const Vec& vec, std::ostream& os, int curr_indent, int indent, bool key)
	{
		using T = typename Vec::value_type;
		os << "{\n";
		for ( size_t i = 0; i < vec.size(); ++i )
		{
			os << std::string(curr_indent + indent, ' ');
			ToLuaString<T>(vec[i], os, curr_indent + indent, indent, false);
			if ( i < vec.size() - 1 ) os << ",";
			os << "\n";
		}
		os << std::string(curr_indent, ' ') << "}";
	}

	// std::map<K, T>
	template <typename Map>
		requires is_specialization_of_v<Map, std::map>
	void ToLuaString(const Map& map, std::ostream& os, int curr_indent, int indent, bool key)
	{
		using K = typename Map::key_type;
		using T = typename Map::mapped_type;
		os << "{\n";
		for ( auto& [k, v] : map )
		{
			os << std::string(curr_indent + indent, ' ');
			os << ToLuaString<K>(k, os, curr_indent + indent, indent, true);
			os << " = ";
			os << ToLuaString<T>(v, os, curr_indent + indent, indent, false);
			os << ",\n";
		}
		os << std::string(curr_indent, ' ') << "}";
	}

	// For structs registered with visit_struct
	template <typename T>
		requires visit_struct::traits::is_visitable<T>::value
	void ToLuaString(const T& t, std::ostream& os, int curr_indent, int indent, bool key)
	{
		os << "{\n";
		visit_struct::for_each(t, [&](const char* name, const auto& value)
		{
			using FieldType = std::decay_t<decltype(value)>;
			os << std::string(curr_indent + indent, ' ');
			os << name << " = ";
			ToLuaString<FieldType>(value, os, curr_indent + indent, indent, false);
			os << ",\n";
		});
		os << std::string(curr_indent, ' ') << "}";
	}

	template <typename T>
	void ToLuaScript(const T& t, std::ostream& os, int indent = 2)
	{
		os << "return ";
		ToLuaString<T>(t, os, 0, indent, false);
	}

	// ============================================================
	// Runtime functions
	// ============================================================

	template <typename T>
	std::function<Ref<void>(const sol::object& obj)> GetFromLuaFunction()
	{
		return [](const sol::object& obj) -> Ref<void> {
			auto tbl = obj.as<sol::table>();
			Ref<T> t = CreateScope<T>();

			visit_struct::for_each(*t, [&](const char* name, auto& value)
			{
				if ( tbl[name].valid() )
				{
					using FieldType = std::decay_t<decltype(value)>;
					value = FromLua<FieldType>(tbl[name]);
				}
			});

			return StaticRefCast<void>(t);
		};
	}

	template <typename T>
	std::function<void(const void* valuePtr, std::ostream& os, int indent)> GetToLuaScriptFunction()
	{
		return [](const void* valuePtr, std::ostream& os, int indent)
		{
			const T& value = *static_cast<const T*>(valuePtr);
			ToLuaScript<T>(value, os, indent);
		};
	}
}
