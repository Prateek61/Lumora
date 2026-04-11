#include "Glimmer.h"
#include <GLM/gtc/matrix_transform.hpp>

void Glimmer::Build(Core::Application& app)
{
	LM_PROFILE_FUNCTION();

	auto& world = app.GetWorld();
	auto& flecs_world = world.Raw();

	// Create Renderer2D Resource
	 m_Renderer2D.Init(*world.GetResource<Lumen::RenderDeviceResource>().Resource);

	auto system = flecs_world.system("Glimmer::Update");
	system.kind(world.Raw().entity<Aether::Phases::OnUpdate>());
	system.read<Flux::KeyboardState>().read<Flux::KeyboardState>().read<Lumen::RenderDeviceResource>();
	system.run([](flecs::iter& iter)
	{
		auto world = Aether::World{iter.world()};
		auto input = Flux::Input::Get(world);
		auto& render_device = world.GetResource<Lumen::RenderDeviceResource>().Resource;
		static float time = 0.0f;
		float dt = iter.delta_time();
		time += dt;
		if (input.Keyboard.Down(Flux::Key::Space))
		{
			float time_normalized = (time / 2.0f);
			float r = (std::sin(time_normalized) + 1.0f) / 2.0f;
			float g = (std::sin(time_normalized + 2.0f) + 1.0f) / 2.0f;
			float b = (std::sin(time_normalized + 4.0f) + 1.0f) / 2.0f;
			render_device->SetClearColor({ r, g, b, 1.0f });
		}
		if (input.Keyboard.Released(Flux::Key::Space))
		{
			render_device->SetClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
		}
	});

	auto system_render = flecs_world.system("Glimmer::Render");
	system_render.kind(world.Raw().entity<Aether::Phases::PreStore>());
	system_render.run([this](flecs::iter& iter)
	{
		static glm::mat4 vp = glm::scale(glm::mat4(1.0f), { 0.5f, 0.5f, 1.0f });

		this->m_Renderer2D.Begin(vp);
		this->m_Renderer2D.DrawQuad({ -0.5f, -0.5f }, { 1.0f, 1.0f }, { 1.0f, 1.0f, 0.0f, 1.0f });
		this->m_Renderer2D.End();
	});
}

void Glimmer::Cleanup(Core::Application& app)
{
	LM_PROFILE_FUNCTION();

	 m_Renderer2D.Shutdown();
}
