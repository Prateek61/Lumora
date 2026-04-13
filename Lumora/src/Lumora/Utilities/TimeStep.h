#pragma once

namespace Lumora
{
	struct TimeStep
	{
	public:
		TimeStep(float timeSeconds = 0.0f)
			: m_Time(timeSeconds)
		{
		}

		float GetSeconds() const { return m_Time; }
		float GetMilliseconds() const { return m_Time * 1000.0f; }

		// Operator overloads
		TimeStep operator+(const TimeStep& other) const { return {m_Time + other.m_Time}; }
		TimeStep operator-(const TimeStep& other) const { return {m_Time - other.m_Time}; }

		TimeStep& operator+=(const TimeStep& other)
		{
			m_Time += other.m_Time;
			return *this;
		}

		TimeStep& operator-=(const TimeStep& other)
		{
			m_Time -= other.m_Time;
			return *this;
		}

	private:
		float m_Time;
	};
}
