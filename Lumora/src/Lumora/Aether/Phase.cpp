#include "LMPCH.h"
#include "Phase.h"

namespace
{
	template <typename Phase>
	void AnchorPhase(flecs::world& world, flecs::entity_t phase_id)
	{
		world.component<Phase>()
			.add(flecs::Phase)
			.depends_on(phase_id);
	}
}

namespace Lumora::Aether
{
	void Phases::Register(World& world)
	{
		// Anchor each of the default Phases to flecs build in Phases.
		AnchorPhase<OnLoad>(world.Raw(), flecs::OnLoad);
		AnchorPhase<PostLoad>(world.Raw(), flecs::PostLoad);
		AnchorPhase<PreUpdate>(world.Raw(), flecs::PreUpdate);
		AnchorPhase<OnUpdate>(world.Raw(), flecs::OnUpdate);
		AnchorPhase<OnValidate>(world.Raw(), flecs::OnValidate);
		AnchorPhase<PostUpdate>(world.Raw(), flecs::PostUpdate);
		AnchorPhase<PreStore>(world.Raw(), flecs::PreStore);
		AnchorPhase<OnStore>(world.Raw(), flecs::OnStore);
	}
}

