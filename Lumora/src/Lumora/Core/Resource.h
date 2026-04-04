#pragma once

#include "Lumora/Utilities/TimeStep.h"

namespace Lumora::Core
{
	struct ApplicationState
	{
		bool Running = true;
	};

	struct DeltaTime: public TimeStep
	{
		DeltaTime(float timeSeconds = 0.0f)
			: TimeStep(timeSeconds)
		{
		}
	};
}