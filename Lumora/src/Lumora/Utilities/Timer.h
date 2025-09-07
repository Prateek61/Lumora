#pragma once

#include <chrono>

namespace Lumora
{
	class Timer
	{
	public:
		Timer()
		{
			Reset();
		}

		void Reset()
		{
			m_Start = std::chrono::high_resolution_clock::now();
		}

		float ElapsedSeconds() const
		{
			auto end = std::chrono::high_resolution_clock::now();
			return static_cast<float>(std::chrono::duration_cast<std::chrono::nanoseconds>(end - m_Start).count()) *
					0.001f * 0.001f * 0.001f;
		}

		float ElapsedMilliseconds() const
		{
			auto end = std::chrono::high_resolution_clock::now();
			return static_cast<float>(std::chrono::duration_cast<std::chrono::nanoseconds>(end - m_Start).count()) *
					0.001f * 0.001f;
		}

	private:
		std::chrono::time_point<std::chrono::high_resolution_clock> m_Start;
	};
}
