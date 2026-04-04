#pragma once

namespace Lumora
{
	class Time
	{
	public:
		static double Get()
		{
			using clock = std::chrono::steady_clock;
			return std::chrono::duration<double>(clock::now() - GetStartTime()).count();
		}

		static float GetF()
		{
			using clock = std::chrono::steady_clock;
			return static_cast<float>(std::chrono::duration<double>(clock::now() - GetStartTime()).count());
		}

		static std::chrono::time_point<std::chrono::steady_clock> GetStartTime()
		{
			static const auto startTime = std::chrono::steady_clock::now();
			return startTime;
		}
	};
}