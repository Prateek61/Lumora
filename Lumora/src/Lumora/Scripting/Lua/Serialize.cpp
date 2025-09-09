#include "LMPCH.h"
#include "Serialize.h"

namespace Lumora::Serialize
{
	void TableToLuaString(const sol::table& table, std::ostream& out, int curr_indent, int indent)
	{
		for (auto& kv: table)
		{
			std::string key = kv.first.as<std::string>();
			sol::object value = kv.second;

			if (value.is<sol::table>())
			{
				out << std::string(curr_indent, ' ') << key << " = {\n";
				TableToLuaString(value, out, curr_indent + indent, indent);
				out << std::string(curr_indent, ' ') << "}\n";
			}
			else
			{
				out << std::string(curr_indent + indent, ' ') << key << " = ";
			}
		}
	}
}