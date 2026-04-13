#include "LMPCH.h"
#include "Renderer2DPlugin.h"

#include "RendererPlugin.h"
#include "Lumora/Core/Application.h"
#include "Lumora/Lumen/Renderer2D.h"

namespace Lumora::Lumen
{
	void Renderer2DPlugin::Build(Core::Application& app)
	{
		auto& world = app.GetWorld();

		// 1. Initialize and Register Renderer2D Resources
		auto renderer_2d = Renderer2D();
		renderer_2d.Init(*world.GetResourceMut<RenderDeviceResource>().Resource);
		world.SetResource(std::move(renderer_2d));

		world.SetResource<Renderer2D::Stats>({});
		world.Raw()
		     .component<Renderer2D::Stats>()
		     .member("DrawCalls", &Renderer2D::Stats::DrawCalls)
		     .member("QuadCount", &Renderer2D::Stats::QuadCount);

		// 2. Create Systems for Renderer2D lifecycle
		auto begin_frame_system = world.System("Renderer2D::BeginFrame");
		begin_frame_system.SetPhase<Aether::Phases::PreRender>().Write<Renderer2D>();
		m_BeginFrameSystem = begin_frame_system.Run([](Aether::QueryRes& res)
		{
			auto& renderer_2d = res.World().GetResourceMut<Renderer2D>();
			renderer_2d.Begin(glm::mat4(1.0f)); // TODO: Pass actual view-projection matrix
		});

		auto end_frame_system = world.System("Renderer2D::EndFrame");
		end_frame_system.SetPhase<Aether::Phases::PostRender>().Write<Renderer2D>().Write<Renderer2D::Stats>();
		m_EndFrameSystem = end_frame_system.Run([](Aether::QueryRes& res)
		{
			auto& renderer_2d = res.World().GetResourceMut<Renderer2D>();
			renderer_2d.End();
		});

		auto update_stats_system = world.System("Renderer2D::UpdateStats");
		update_stats_system.SetPhase<Aether::Phases::PostRender>().Read<Renderer2D>().Write<Renderer2D::Stats>();
		m_UpdateStatsSystem = update_stats_system.Run([](Aether::QueryRes& res)
		{
			auto world = res.World();
			auto& renderer_2d = world.GetResourceMut<Renderer2D>();
			auto& stats = world.GetResourceMut<Renderer2D::Stats>();
			stats = renderer_2d.GetStats();
		});

		// A Flush system for manual flushing if needed, not attached to any phase so not automatically run
		auto flush_system = world.System("Renderer2D::Flush").Write<Renderer2D>();
		m_FrameFlushSystem = flush_system.Run([](Aether::QueryRes& res)
		{
			auto& renderer_2d = res.World().GetResourceMut<Renderer2D>();
			renderer_2d.Flush();
		});
	}

	void Renderer2DPlugin::Cleanup(Core::Application& app)
	{
		LM_PROFILE_FUNCTION();

		app.GetWorld().GetResourceMut<Renderer2D>().Shutdown();
	}

	void Renderer2DPlugin::AddDependencies(Core::DependencyList& dependencies)
	{
		dependencies.Require<RendererPlugin>();
	}
}
