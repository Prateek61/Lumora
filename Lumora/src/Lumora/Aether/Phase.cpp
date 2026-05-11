#include "LMPCH.h"
#include "Phase.h"

namespace Lumora::Aether
{
	PhaseBuilder::PhaseBuilder(World& world, Entity phase)
		: m_World(&world), m_PhaseEntity(phase.Raw())
	{
		m_PhaseEntity.add(flecs::Phase);
	}

	PhaseBuilder& PhaseBuilder::DependsOn(Entity dependency)
	{
		LM_CORE_ASSERT(dependency.Raw().is_valid() && dependency.Raw().has(flecs::Phase),
		               "Dependency phase does not exist or is not a phase.");

		m_PhaseEntity.depends_on(dependency.Raw());
		return *this;
	}

	PhaseBuilder& PhaseBuilder::Before(Entity target)
	{
		LM_CORE_ASSERT(target.Raw().is_valid() && target.Raw().has(flecs::Phase), "Target phase does not exist or is not a phase.");

		target.Raw().depends_on(m_PhaseEntity);
		return *this;
	}

	void Phases::Register(World& world)
	{
		LM_PROFILE_FUNCTION();

		// Flecs anchor Phases
		auto& f_world = world.Raw();
		auto f_pre_frame = Entity(f_world.entity(flecs::PreFrame));
		auto f_post_frame = Entity(f_world.entity(flecs::PostFrame));

		auto frame_start = world.AddPhase<FrameStart>().DependsOn(f_pre_frame);
		auto input = world.AddPhase<Input>().DependsOn<FrameStart>();
		auto pre_update = world.AddPhase<PreUpdate>().DependsOn<Input>();
		auto update = world.AddPhase<OnUpdate>().DependsOn<PreUpdate>();
		auto on_validate = world.AddPhase<OnValidate>().DependsOn<OnUpdate>();
		auto post_update = world.AddPhase<PostUpdate>().DependsOn<OnValidate>();
		auto pre_render = world.AddPhase<PreRender>().DependsOn<PostUpdate>();
		auto render = world.AddPhase<OnRender>().DependsOn<PreRender>();
		auto post_render = world.AddPhase<PostRender>().DependsOn<OnRender>();
		auto on_ui = world.AddPhase<OnUI>().DependsOn<PostRender>();
		auto post_ui = world.AddPhase<PostUI>().DependsOn<OnUI>();
		auto present = world.AddPhase<Present>().DependsOn<PostUI>();
		auto frame_end = world.AddPhase<FrameEnd>().DependsOn<Present>().Before(f_post_frame);

		frame_start.Before<Input>();
		input.Before<PreUpdate>();
		pre_update.Before<OnUpdate>();
		update.Before<OnValidate>();
		on_validate.Before<PostUpdate>();
		post_update.Before<PreRender>();
		pre_render.Before<OnRender>();
		render.Before<PostRender>();
		post_render.Before<OnUI>();
		on_ui.Before<PostUI>();
		post_ui.Before<Present>();
		present.Before<FrameEnd>();
	}
}
