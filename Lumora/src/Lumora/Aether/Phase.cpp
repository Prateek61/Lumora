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

		world.AddPhase<FrameStart>().DependsOn(f_pre_frame);
		world.AddPhase<Input>().DependsOn<FrameStart>();
		world.AddPhase<PreUpdate>().DependsOn<Input>();
		world.AddPhase<OnUpdate>().DependsOn<PreUpdate>();
		world.AddPhase<OnValidate>().DependsOn<OnUpdate>();
		world.AddPhase<PostUpdate>().DependsOn<OnValidate>();
		world.AddPhase<PreRender>().DependsOn<PostUpdate>();
		world.AddPhase<OnRender>().DependsOn<PreRender>();
		world.AddPhase<PostRender>().DependsOn<OnRender>();
		world.AddPhase<OnUI>().DependsOn<PostRender>();
		world.AddPhase<PostUI>().DependsOn<OnUI>();
		world.AddPhase<Present>().DependsOn<PostUI>();
		world.AddPhase<FrameEnd>().DependsOn<Present>().Before(f_post_frame);
	}
}
