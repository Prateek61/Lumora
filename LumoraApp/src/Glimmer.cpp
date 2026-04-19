#include "Glimmer.h"
#include <GLM/gtc/matrix_transform.hpp>
#include <imgui.h>

#include <inttypes.h>
#include <numbers>

namespace
{
	void Update(Aether::QueryRes& res)
	{
		auto world = res.World();
		auto input = Flux::Input::Get(world);
		auto& render_device = world.GetResource<Lumen::RenderDeviceResource>().Resource;
		static float time = 0.0f;
		time += res.DeltaTime().GetSeconds();
		if (input.Keyboard.Down(Flux::Key::Space))
		{
			float time_normalized = (time / 1.0f);
			float r = (std::sin(time_normalized) + 1.0f) / 4.0f + 0.25f;
			float g = (std::sin(time_normalized + 2.0f) + 1.0f) / 4.0f + 0.25f;
			float b = (std::sin(time_normalized + 4.0f) + 1.0f) / 4.0f + 0.25f;
			render_device->SetClearColor({r, g, b, 1.0f});
		}
		if (input.Keyboard.Released(Flux::Key::Space))
		{
			render_device->SetClearColor({0.1f, 0.1f, 0.1f, 1.0f});
		}
	}

	void Render(Aether::QueryRes& res)
	{
		auto& renderer_2d = res.World().GetResourceMut<Lumen::Renderer2D>();

		static float time = 0.0f;
		constexpr float pi = std::numbers::pi_v<float>;
		time += res.DeltaTime().GetSeconds();

		constexpr int gridSize = 50;
		constexpr float quadSize = 0.03f;
		constexpr float gap = 0.01f;
		constexpr float step = quadSize + gap;

		float cx = 0.0f;
		float cy = 0.0f;

		for (int y = 0; y < gridSize; y++)
		{
			for (int x = 0; x < gridSize; x++)
			{
				float xpos = (x - gridSize / 2.0f) * step;
				float ypos = (y - gridSize / 2.0f) * step;

				float dx = xpos - cx;
				float dy = ypos - cy;
				float dist = std::sqrt(dx * dx + dy * dy);
				float angle = std::atan2(dy, dx);

				// Ripple wave expanding outward
				float wave = std::sin(dist * 8.0f - time * 3.0f) * 0.5f + 0.5f;

				// Spiral color rotation
				float spiral = std::sin(angle * 3.0f + dist * 4.0f - time * 2.0f) * 0.5f + 0.5f;

				// Pulsing scale based on distance
				float pulse = std::sin(dist * 6.0f - time * 4.0f) * 0.3f + 0.7f;
				float size = quadSize * pulse;

				// Offset position so quads pulse from their center
				float offset = (quadSize - size) * 0.5f;

				// Color palette - shifting hues based on wave, spiral, and time
				float r = std::sin(wave * pi + time * 0.7f) * 0.4f + 0.6f;
				float g = std::sin(spiral * pi + time * 0.5f + 2.0f) * 0.4f + 0.5f;
				float b = std::sin((wave + spiral) * pi + time * 0.3f + 4.0f) * 0.4f + 0.6f;

				// Fade out edges
				float fade = 1.0f - std::clamp(dist / 1.0f, 0.0f, 1.0f);
				fade = fade * fade; // quadratic falloff

				float alpha = fade * (0.6f + wave * 0.4f);

				if (alpha > 0.02f)
				{
					renderer_2d.DrawQuad(
						{xpos + offset, ypos + offset},
						{size, size},
						{r, g, b, alpha}
					);
				}
			}
		}
	}

	void UI(Aether::QueryRes& res)
	{
		const auto& stats = res.World().GetResource<Lumen::Renderer2D::Stats>();

		ImGui::Begin("Draw Calls");

		ImGui::Text("Draw Calls: %" PRIu32, stats.DrawCalls);
		ImGui::Text("Quad Count: %" PRIu32, stats.QuadCount);

		ImGui::End();
	}
}

void Glimmer::Build(Core::Application& app)
{
	LM_PROFILE_FUNCTION();

	auto& world = app.GetWorld();

	auto update_system_builder = world.System("Glimmer::Update");
	update_system_builder.SetPhase<Aether::Phases::OnUpdate>().Read<Flux::KeyboardState>().Read<Flux::MouseState>().Read<
		Lumen::RenderDeviceResource>();
	m_UpdateSystem = update_system_builder.Run(Update);

	auto render_system_builder = world.System("Glimmer::Render");
	render_system_builder.SetPhase<Aether::Phases::OnRender>().Write<Lumen::Renderer2D>();
	m_RenderSystem = render_system_builder.Run([this](Aether::QueryRes& res)
	{
		Render(res);
	});

	auto ui_system_builder = world.System("Glimmer::UI");
	ui_system_builder.SetPhase<Aether::Phases::OnUI>().Read<Lumen::Renderer2D::Stats>();
	ui_system_builder.Run(UI);
}

void Glimmer::Cleanup(Core::Application& app)
{
}

void Glimmer::AddDependencies(Core::DependencyList& dependencies)
{
	dependencies.Require<Flux::WindowPlugin>();
	dependencies.Require<Lumen::RendererPlugin>();
	dependencies.Require<Lumen::Renderer2DPlugin>();
}
