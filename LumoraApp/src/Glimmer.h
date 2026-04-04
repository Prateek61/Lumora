#pragma once

#include "Lumora.h"

using namespace Lm;

struct IterationCount
{
	int count = 0;
};

class Glimmer : public Core::Plugin
{
public:
	Glimmer() = default;
	
	void Build(Core::Application& app) override
	{
		auto& world = app.GetWorld();
		auto& flecs_world = world.Raw();

		// .set<T>() on the world creates the singleton AND assigns a value.
		// .component<T>().add(flecs::Singleton) only registers metadata
		// it doesn't create an instance with data for systems to match.
		flecs_world.set<IterationCount>({ 0 });

		// Add Systems
		flecs_world.system<const IterationCount>("OnLoad")
			.kind(flecs_world.component<Aether::Phases::OnLoad>())
			.each([](const IterationCount& count)
				{
					LM_LOG_INFO("Glimmer: OnLoad System Running! Iteration: {}", count.count);
				});

		
		flecs_world.system<const IterationCount>("OnUpdate")
			.kind(flecs_world.component<Aether::Phases::OnUpdate>())
			.each([](const IterationCount& count)
				{
					LM_LOG_INFO("Glimmer: OnUpdate System Running! Iteration: {}", count.count);
				});

		flecs_world.system<const IterationCount>("PostUpdate")
			.kind(flecs_world.component<Aether::Phases::PostUpdate>())
			.each([](const IterationCount& count)
				{
					LM_LOG_INFO("Glimmer: PostUpdate System Running! Iteration: {}", count.count);
				});

		flecs_world.system<IterationCount>("PreStore")
			.kind(flecs_world.component<Aether::Phases::PreStore>())
			.run([](flecs::iter& it)
				{
					if (!it.next())
					{
						LM_CORE_WARN("Glimmer: No entities with IterationCount component found. Quitting application.");
						it.world().quit();
						return;
					}

					auto c_arr = it.field<IterationCount>(0);

					c_arr[0].count++;
					if (c_arr[0].count > 5)
					{
						LM_LOG_INFO("Glimmer: Reached iteration limit, quitting application.");
						it.world().quit();
					}
					it.fini();
				});
	}

	const char* GetName() const override
	{
		return "Glimmer";
	}
};