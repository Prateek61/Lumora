#include "LMPCH.h"
#include "Time.h"

#include "GLFW/glfw3.h"

namespace Lumora
{
	double Time::Get()
	{
		return glfwGetTime();
	}

	float Time::GetF()
	{
		return static_cast<float>(glfwGetTime());
	}
}