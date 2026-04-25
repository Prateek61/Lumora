#include "LMPCH.h"
#include "LuaSerializerPlugin.h"

#include "Lumora/Core/Application.h"

namespace Lumora::Rune
{
	void LuaSerializerPlugin::Build(Core::Application& app)
	{
		LM_PROFILE_FUNCTION();

		auto& world = app.GetWorld();

		// Set Resource
		world.SetResource<LuaSerializerResource>({.Resource = CreateScope<LuaSerializer>()});
	}

	void LuaSerializerPlugin::Cleanup(Core::Application& app)
	{
		LM_PROFILE_FUNCTION();

		auto& world = app.GetWorld();

		// Remove Resource, Calls the relevant destructor
		world.GetResourceMut<LuaSerializerResource>().Resource.reset();
		world.RemoveResource<LuaSerializerResource>();
	}
}
