#pragma once

#include "Lumora/Common/Threading.h"
#include "Lumora/Scripting/Lua/LuaBase.h"
#include "Lumora/Core/Props.h"

namespace Lumora
{
	class ConfigReader
	{
		// Class that wraps sol::state
		// Provides an interface to read configuration files
	public:
		// Public for external locking
		LM_MUTEX_AUTO()

	public:
		ConfigReader();

		// Not Thread Safe
		sol::state& GetLuaState() { return m_Lua; }

		ApplicationProps LoadAppProps(const std::filesystem::path& file);

		template<typename T>
		T LoadStruct(const std::filesystem::path& file);
	private:
		sol::state m_Lua;
	};
}

// Template Implementations
namespace Lumora
{
	template<typename T>
	T ConfigReader::LoadStruct(const std::filesystem::path& file)
	{
		LM_LOCK_READ_AUTO();

		if (!std::filesystem::exists(file))
		{
			throw std::runtime_error("Config file does not exist: " + file.string());
		}

		// Load and execute the Lua script
		auto result = m_Lua.script_file(file.string());
		if (!result.valid())
		{
			sol::error err = result;
			throw std::runtime_error("Failed to load config file: " + std::string(err.what()));
		}

		T instance{};

		// Try to get a global table with the struct name or "config"
		sol::optional<sol::table> configTable = m_Lua["config"];
		if (!configTable)
		{
			// Fallback to looking for a table with the type name
			std::string typeName = typeid(T).name();
			configTable = m_Lua[typeName];
		}

		if (configTable)
		{
			// Use reflection or manual mapping to populate the struct
			// This assumes T has a sol usertype binding or can be constructed from the table
			if constexpr (std::is_constructible_v<T, sol::table>)
			{
				instance = T(*configTable);
			}
			else
			{
				// Try to convert the table directly to T
				sol::optional<T> converted = configTable->as<sol::optional<T>>();
				if (converted)
				{
					instance = *converted;
				}
			}
		}

		return instance;
	}
}
