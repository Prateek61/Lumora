#include "LMPCH.h"
#include "Input.h"

#include "Lumora/Aether/World.h"

namespace Lumora::Flux
{
	Input Input::Get(Aether::World& world)
	{
		return Input(world.GetResource<KeyboardState>(), world.GetResource<MouseState>());
	}
}