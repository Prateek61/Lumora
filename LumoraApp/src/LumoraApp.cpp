#include "Lumora.h"
#include <iostream>

using namespace Lm;

struct Position
{
	float x, y;
};

struct Velocity
{
	float x, y;
};

int main()
{
	// std::cout << "Hello, Lumora!" << std::endl;
	Aether::World world;
	flecs::world& flecs_world = world.Raw();

	flecs_world.system<Position, const Velocity>("Update")
	           .each([](Position& pos, const Velocity& v)
	           {
		           pos.x += v.x;
		           pos.y += v.y;
	           });

	// Logger syste
	flecs_world.system<const Position, const Velocity>("Logger")
			   .each([](const Position& pos, const Velocity& v)
			   {
					   std::cout << "Position: (" << pos.x << ", " << pos.y << ")"
						   << " Velocity: (" << v.x << ", " << v.y << ")"
						   << "\n";
				   });

	auto e = world.CreateEntity("Player").Raw().insert([](Position& p, Velocity& v)
		{
			p = { 10, 20 };
			v = { 1, 2 };
		});

	while (flecs_world.progress()) {}
}
