#pragma once

#pragma once

#include "Lumora/Common/Defines.h"

#include <chrono>
#include <fstream>
#include <string>
#include <thread>
#include <mutex>

namespace Lumora
{
	using FloatingPointMicroseconds = std::chrono::duration<double, std::micro>;

	struct ProfileResult
	{
		std::string Name;

		FloatingPointMicroseconds Start;
		std::chrono::microseconds ElapsedTime;
		std::thread::id ThreadID;
	};

	struct InstrumentationSession
	{
		std::string Name;
	};

	class Instrumentor
	{
	public:
		Instrumentor(const Instrumentor&) = delete;
		Instrumentor(Instrumentor&&) = delete;

		void BeginSession(const std::string& name, const std::string& filepath = "results.json");
		void EndSession();
		void WriteProfile(const ProfileResult& result);
		static Instrumentor& Get();
		static bool IsProfilingEnabled();
		static void EnableProfiling();
		static void DisableProfiling();

	private:
		Instrumentor();
		~Instrumentor();
		void WriteHeader();
		void WriteFooter();

		// Note: you must already own lock on m_Mutex before
		// calling InternalEndSession()
		void InternalEndSession();

		std::mutex m_Mutex;
		InstrumentationSession* m_CurrentSession;
		std::ofstream m_OutputStream;
		bool m_ProfilingEnabled = true;
	};


	class InstrumentationTimer
	{
	public:
		InstrumentationTimer(const char* name);
		~InstrumentationTimer();
		void Stop();

	private:
		const char* m_Name;
		std::chrono::time_point<std::chrono::steady_clock> m_StartTimePoint;
		bool m_Stopped;
	};


	namespace InstrumentorUtils
	{
		template <size_t N>
		struct ChangeResult
		{
			char Data[N];
		};

		template <size_t N, size_t K>
		constexpr auto CleanupOutputString(const char (&expr)[N], const char (&remove)[K])
		{
			ChangeResult<N> result = {};

			size_t src_index = 0;
			size_t dst_index = 0;
			while ( src_index < N )
			{
				size_t match_index = 0;
				while ( match_index < K - 1 && src_index + match_index < N - 1 && expr[src_index + match_index] ==
					remove[match_index] )
					match_index++;
				if ( match_index == K - 1 ) src_index += match_index;
				result.Data[dst_index++] = expr[src_index] == '"' ? '\'' : expr[src_index];
				src_index++;
			}

			return result;
		}
	}
}

#ifdef LM_ENABLE_PERFORMANCE_PROFILING
#define LM_PROFILE_BEGIN_SESSION(name, filepath) ::Lumora::Instrumentor::Get().BeginSession(name, filepath)
#define LM_PROFILE_END_SESSION() ::Lumora::Instrumentor::Get().EndSession()
#define LM_PROFILE_SCOPE_LINE2(name, line) constexpr auto fixedName##line = ::Lumora::InstrumentorUtils::CleanupOutputString(name, "__cdecl ");\
												   ::Lumora::InstrumentationTimer timer##line(fixedName##line.Data)
#define LM_PROFILE_SCOPE_LINE(name, line) LM_PROFILE_SCOPE_LINE2(name, line)
#define LM_PROFILE_SCOPE(name) LM_PROFILE_SCOPE_LINE(name, __LINE__)
#define LM_PROFILE_FUNCTION() LM_PROFILE_SCOPE(LM_FUNC_SIG)

#define LM_PROFILE_ENABLE() ::Lumora::Instrumentor::EnableProfiling()
#define LM_PROFILE_DISABLE() ::Lumora::Instrumentor::DisableProfiling()
#else
#define LM_PROFILE_BEGIN_SESSION(name, filepath)
#define LM_PROFILE_END_SESSION()
#define LM_PROFILE_SCOPE(name)
#define LM_PROFILE_FUNCTION()

#define LM_PROFILE_ENABLE()
#define LM_PROFILE_DISABLE()
#endif
