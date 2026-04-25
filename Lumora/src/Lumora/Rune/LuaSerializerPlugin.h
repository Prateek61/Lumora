#pragma once

#include "Lumora/Rune/LuaSerializer.h"
#include "Lumora/Core/Plugin.h"

namespace Lumora::Rune
{
	using LuaSerializerResource = ScopedResource<LuaSerializer>;

	class LuaSerializerPlugin final : public Core::Plugin
	{
	public:
		void Build(Core::Application& app) override;
		void Cleanup(Core::Application& app) override;

		const char* GetName() const override { return "LuaSerializerPlugin"; }
	};
}
