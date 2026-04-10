#include "Glimmer.h"

void Glimmer::Build(Core::Application& app)
{
	auto& world = app.GetWorld();
	auto& flecs_world = world.Raw();

	auto system = flecs_world.system("GlimmerSystem");;
	system.kind(world.Raw().entity<Aether::Phases::OnUpdate>());
	system.read<Flux::KeyboardState>().read<Flux::KeyboardState>();
	system.run([](flecs::iter& iter)
		{
			auto world = Aether::World{ iter.world() };
			auto input = Flux::Input::Get(world);

			if (input.Keyboard.Pressed(Flux::Key::Space))
			{
				LM_LOG_INFO("Space key was just pressed!");
			}
			if (input.Keyboard.Down(Flux::Key::Space))
			{
				LM_LOG_INFO("Space Key is Held Down!");
			}
			if (input.Keyboard.Released(Flux::Key::Space))
			{
				LM_LOG_INFO("Space Key was just released!");
			}
			iter.world().quit();
		});
}
