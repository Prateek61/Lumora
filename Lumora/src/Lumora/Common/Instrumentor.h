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
// Resolve which function signature macro will be used. Note that this only
// is resolved when the (pre)compiler starts, so the syntax highlighting
// could mark the wrong one in your editor!
#if defined(__GNUC__) || (defined(__MWERKS__) && (__MWERKS__ >= 0x3000)) || (defined(__ICC) && (__ICC >= 600)) || defined(__ghs__)
#define LM_FUNC_SIG __PRETTY_FUNCTION__
#elif defined(__DMC__) && (__DMC__ >= 0x810)
#define LM_FUNC_SIG __PRETTY_FUNCTION__
#elif (defined(__FUNCSIG__) || (_MSC_VER))
#define LM_FUNC_SIG __FUNCSIG__
#elif (defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 600)) || (defined(__IBMCPP__) && (__IBMCPP__ >= 500))
#define LM_FUNC_SIG __FUNCTION__
#elif defined(__BORLANDC__) && (__BORLANDC__ >= 0x550)
#define LM_FUNC_SIG __FUNC__
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901)
#define LM_FUNC_SIG __func__
#elif defined(__cplusplus) && (__cplusplus >= 201103)
#define LM_FUNC_SIG __func__
#else
#define LM_FUNC_SIG "LM_FUNC_SIG unknown!"
#endif

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
