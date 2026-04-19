#pragma once

#include "Lumora/Aether/Base.h"

namespace Lumora::Aether
{
	class PhaseBuilder
	{
	public:
		explicit PhaseBuilder(World& world, Entity phase);

		template <typename Dependency>
		PhaseBuilder& DependsOn();
		PhaseBuilder& DependsOn(Entity dependency);

		template <typename Target>
		PhaseBuilder& Before();
		PhaseBuilder& Before(Entity target);

	private:
		World* m_World;
		flecs::entity m_PhaseEntity;
	};

	namespace Phases
	{
		struct None {};		  // Mark system as not running in any phase. Use for manual control or one-off systems.
		struct FrameStart {}; // Internal anchor: marks the beginning of a frame. Not for user systems.
		struct Input {};      // Poll OS events, update keyboard/mouse/gamepad state.
		struct PreUpdate {};  // Pre-logic setup: timers, animation sampling, network receive.
		struct OnUpdate {};   // Main game logic: AI, movement, physics, gameplay rules.
		struct OnValidate {}; // Validate state after logic: constraint checks, transform propagation.
		struct PostUpdate {}; // React to validated state: collision response, trigger events, cleanup.
		struct PreRender {};  // Begin GPU frame: clear buffers, bind camera, prepare render state.
		struct OnRender {};   // Submit draw calls: sprites, meshes, particles, text.
		struct PostRender {}; // Flush render batches to GPU, post-processing effects.
		struct OnUI {};       // UI rendering: ImGui windows, debug overlays, editor panels.
		struct PostUI {};     // Finalize UI: ImGui::EndFrame/Render, submit UI draw data to GPU.
		struct Present {};    // Present final image: swap buffers, frame complete.
		struct FrameEnd {};   // Internal anchor: marks the end of a frame. Not for user systems.


		void Register(World& world);
	}
}

#include "Lumora/Aether/World.h"

// Template Implementation
namespace Lumora::Aether
{
	template <typename Dependency>
	PhaseBuilder& PhaseBuilder::DependsOn()
	{
		auto dependency_entity = m_World->Raw().entity<Dependency>();

		LM_CORE_ASSERT(dependency_entity.is_valid() && dependency_entity.has(flecs::Phase),
		               "Dependency phase does not exist or is not a phase.");

		m_PhaseEntity.depends_on(dependency_entity);
		return *this;
	}

	template <typename Target>
	PhaseBuilder& PhaseBuilder::Before()
	{
		auto target_entity = m_World->Raw().entity<Target>();

		LM_CORE_ASSERT(target_entity.is_valid() && target_entity.has(flecs::Phase), "Target phase does not exist or is not a phase.");

		target_entity.depends_on(m_PhaseEntity);
		return *this;
	}
}
