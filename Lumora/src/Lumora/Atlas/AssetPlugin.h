#pragma once

#include "Lumora/Core/Plugin.h"
#include "Lumora/Aether/World.h"
#include "Lumora/Aether/System.h"
#include "Lumora/Atlas/AssetServer.h"
#include "Lumora/Atlas/AssetWatcher.h"

namespace Lumora::Atlas
{
	using AssetServerResource = ScopedResource<AssetServer>;
	using AssetWatcherResource = ScopedResource<AssetWatcher>;

	struct AssetSettings
	{
		std::filesystem::path AssetRoot = "Assets/";
		bool HotReload = false;
	};

	class AssetPlugin : public Core::Plugin
	{
	public:
		explicit AssetPlugin(AssetSettings settings = {});

		void Build(Core::Application& app) override;
		void Finish(Core::Application& app) override;
		void Cleanup(Core::Application& app) override;
		void AddDependencies(Core::DependencyList& dependencies) override;

		const char* GetName() const override { return "AssetPlugin"; }

	private:
		AssetSettings m_Settings;

		Aether::System m_WatcherDrainSystem;
		Aether::System m_PumpSystem;
	};
}