#include "LMPCH.h"
#include "Phase.h"

namespace
{
	template <typename Phase>
	void AnchorPhase(flecs::world& world, flecs::entity_t after, flecs::entity_t before)
	{
		
	}
}

namespace Lumora::Aether
{
	PhaseBuilder::PhaseBuilder(World& world, Entity phase)
		: m_World(&world), m_PhaseEntity(phase.Raw())
	{
		m_PhaseEntity.add(flecs::Entity);
	}

	PhaseBuilder& PhaseBuilder::DependsOn(Entity dependency)
	{
		m_PhaseEntity.depends_on(dependency.Raw());
		return *this;
	}

	PhaseBuilder& PhaseBuilder::Before(Entity target)
	{
		target.Raw().depends_on(m_PhaseEntity);
		return *this;
	}

	void Phases::Register(World& world)
	{
		// Anchor each of the default Phases to flecs build in Phases.
		auto& f_world = world.Raw();
		auto f_on_load = Entity(f_world.entity(flecs::OnLoad));
		auto f_post_load = Entity(f_world .entity(flecs::PostLoad));
		auto f_pre_update = Entity(f_world.entity(flecs::PreUpdate));
		auto f_on_update = Entity(f_world.entity(flecs::OnUpdate));
		auto f_on_validate = Entity(f_world.entity(flecs::OnValidate));
		auto f_post_update = Entity(f_world.entity(flecs::PostUpdate));
		auto f_pre_store = Entity(f_world.entity(flecs::PreStore));
		auto f_on_store = Entity(f_world.entity(flecs::OnStore));
		auto f_post_frame = Entity(f_world.entity(flecs::PostFrame));

		world.AddPhase<OnLoad>().DependsOn(f_on_load).Before(f_post_load);
		world.AddPhase<PostLoad>().DependsOn(f_post_load).Before(f_pre_update);
		world.AddPhase<PreUpdate>().DependsOn(f_pre_update).Before(f_on_update);
		world.AddPhase<OnUpdate>().DependsOn(f_on_update).Before(f_on_validate);
		world.AddPhase<OnValidate>().DependsOn(f_on_validate).Before(f_post_update);
		world.AddPhase<PostUpdate>().DependsOn(f_post_update).Before(f_pre_store);
		world.AddPhase<PreStore>().DependsOn(f_pre_store).Before(f_on_store);
		world.AddPhase<OnStore>().DependsOn(f_on_store).Before(f_post_frame);
	}
}

