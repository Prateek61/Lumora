#include "Lumora.h"

#include "Lumora/Core/Props.h"

int main()
{
	Lumora::Log::Init();

	Lumora::LuaSerializer serializer;

	Lumora::ApplicationProps props = serializer.DeserializeFromFile<Lumora::ApplicationProps>("../Assets/Config.local.lua");
	LM_TRACE("Application Props: {0}", serializer.SerializeToLuaScript(props));
}
