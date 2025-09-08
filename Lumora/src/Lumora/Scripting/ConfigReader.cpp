#include "LMPCH.h"
#include "ConfigReader.h"

namespace Lumora
{
	ConfigReader::ConfigReader()
	{
		m_Lua.open_libraries(sol::lib::base, sol::lib::table, sol::lib::string);
	}
}