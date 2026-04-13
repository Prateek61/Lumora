#include "LMPCH.h"
#include "Input.h"

#include "Lumora/Aether/World.h"

namespace Lumora::Flux
{
	Input Input::Get(Aether::World& world)
	{
		LM_PROFILE_FUNCTION();

		return Input(world.GetResource<KeyboardState>(), world.GetResource<MouseState>());
	}
}