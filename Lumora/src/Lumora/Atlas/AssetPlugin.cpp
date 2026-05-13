#include "LMPCH.h"
#include "AssetPlugin.h"

#include "Lumora/Core/Application.h"
#include "Lumora/Aether/World.h"
#include "Lumora/Rune/LuaSerializerPlugin.h"

namespace
{
	using namespace Lumora;

	void WatcherDrain(Aether::QueryRes& res)
	{
		LM_PROFILE_FUNCTION();

		static float time_accumulator = 0.0f;
		time_accumulator += res.DeltaTime().GetSeconds();
		if (time_accumulator < 0.25f)
			return;
		time_accumulator = 0.0f;

		auto& watcher_res = res.World().GetResourceMut<Atlas::AssetWatcherResource>();
		auto& server_res = res.World().GetResourceMut<Atlas::AssetServerResource>();

		watcher_res->Drain([&](const std::filesystem::path& path)
		{
			LM_PROFILE_SCOPE("AssetPlugin: AssetWatcher Drain Callback");
			LM_CORE_INFO("AssetWatcher detected change in '{}'", path.string());

			for (auto id : server_res->LookupByPath(path))
			{
				server_res->QueueReload(id);
			}
		});
	}

	void AssetServerPump(Aether::QueryRes& res)
	{
		LM_PROFILE_FUNCTION();

		auto& server_res = res.World().GetResourceMut<Atlas::AssetServerResource>();
		server_res->Pump();
	}
}

namespace Lumora::Atlas
{
	AssetPlugin::AssetPlugin(AssetSettings settings)
		: m_Settings(std::move(settings)) {}

	void AssetPlugin::Build(Core::Application& app)
	{
		LM_PROFILE_FUNCTION();

		auto& world = app.GetWorld();
		auto& serializer_res = world.GetResourceMut<Rune::LuaSerializerResource>();

		// AssetServer is always present
		world.SetResource<AssetServerResource>({CreateScope<AssetServer>(world, *serializer_res)});

		// AssetWatcher only when hot reload is enabled.
		if (m_Settings.HotReload)
		{
			world.SetResource<AssetWatcherResource>(
				{CreateScope<AssetWatcher>(m_Settings.AssetRoot)});

			m_WatcherDrainSystem = world.System("Atlas::WatcherDrain")
			                  .SetPhase<Aether::Phases::FrameStart>()
			                  .Run(WatcherDrain);
		}

		// Pump asset server reloads at the end of the frame, after all changes have been collected.
		m_PumpSystem = world.System("Atlas::AssetServerPump")
		                 .SetPhase<Aether::Phases::PreUpdate>()
		                 .Run(AssetServerPump);
	}

	void AssetPlugin::Finish(Core::Application& app)
	{
		LM_PROFILE_FUNCTION();

		auto& world = app.GetWorld();
		auto& server_res = world.GetResourceMut<AssetServerResource>();
		server_res->Scan(m_Settings.AssetRoot);
	}

	void AssetPlugin::Cleanup(Core::Application& app)
	{
		LM_PROFILE_FUNCTION();

		auto& world = app.GetWorld();

		if (m_Settings.HotReload)
			m_WatcherDrainSystem.Disable();
		m_PumpSystem.Disable();

		if (auto* w = world.TryGetResourceMut<AssetWatcherResource>())
		{
			w->Resource.reset();
		}
		if (auto* s = world.TryGetResourceMut<AssetServerResource>())
		{
			s->Resource.reset();
		}
	}

	void AssetPlugin::AddDependencies(Core::DependencyList& dependencies)
	{
		dependencies.Require<Rune::LuaSerializerPlugin>();
	}
}
