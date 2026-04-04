#include "LMPCH.h"
#include "Instrumentor.h"

namespace Lumora
{
	void Instrumentor::BeginSession(const std::string& name, const std::string& filepath)
	{
		std::lock_guard lock(m_Mutex);
		if ( m_CurrentSession )
		{
			// If there is already a current session, then close it before beginning new one.
			// Subsequent profiling output meant for the original session will end up in the
			// newly opened session instead.  That's better than having badly formatted
			// profiling output.
			if ( Log::GetCoreLogger() ) // Edge case: BeginSession() might be before Log::Init()
			{
				LM_CORE_ERROR("Instrumentor::BeginSession('{0}') when session '{1}' already open.", name,
				              m_CurrentSession->Name);
			}
			InternalEndSession();
		}
		m_OutputStream.open(filepath);

		if ( m_OutputStream.is_open() )
		{
			m_CurrentSession = new InstrumentationSession({name});
			WriteHeader();
		}
		else
		{
			if ( Log::GetCoreLogger() ) // Edge case: BeginSession() might be before Log::Init()
			{
				LM_CORE_ERROR("Instrumentor could not open results file '{0}'.", filepath);
			}
		}
	}

	void Instrumentor::EndSession()
	{
		std::lock_guard lock(m_Mutex);
		InternalEndSession();
	}

	void Instrumentor::WriteProfile(const ProfileResult& result)
	{
		if ( !m_ProfilingEnabled )
		{
			return;
		}

		std::stringstream json;

		json << std::setprecision(3) << std::fixed;
		json << ",{";
		json << "\"cat\":\"function\",";
		json << "\"dur\":" << (result.ElapsedTime.count()) << ',';
		json << "\"name\":\"" << result.Name << "\",";
		json << "\"ph\":\"X\",";
		json << "\"pid\":0,";
		json << "\"tid\":" << result.ThreadID << ",";
		json << "\"ts\":" << result.Start.count();
		json << "}";

		std::lock_guard lock(m_Mutex);
		if ( m_CurrentSession )
		{
			m_OutputStream << json.str();
			m_OutputStream.flush();
		}
	}

	Instrumentor& Instrumentor::Get()
	{
		static Instrumentor instance;
		return instance;
	}

	bool Instrumentor::IsProfilingEnabled()
	{
		return Get().m_ProfilingEnabled;
	}

	void Instrumentor::EnableProfiling()
	{
		Get().m_ProfilingEnabled = true;
	}

	void Instrumentor::DisableProfiling()
	{
		Get().m_ProfilingEnabled = false;
	}

	Instrumentor::Instrumentor()
		: m_CurrentSession(nullptr)
	{
	}

	Instrumentor::~Instrumentor()
	{
		EndSession();
	}

	void Instrumentor::WriteHeader()
	{
		m_OutputStream << "{\"otherData\": {},\"traceEvents\":[{}";
		m_OutputStream.flush();
	}

	void Instrumentor::WriteFooter()
	{
		m_OutputStream << "]}";
		m_OutputStream.flush();
	}

	void Instrumentor::InternalEndSession()
	{
		if ( m_CurrentSession )
		{
			WriteFooter();
			m_OutputStream.close();
			delete m_CurrentSession;
			m_CurrentSession = nullptr;
		}
	}


	InstrumentationTimer::InstrumentationTimer(const char* name)
		: m_Name(name), m_Stopped(false)
	{
		m_StartTimePoint = std::chrono::steady_clock::now();
	}

	InstrumentationTimer::~InstrumentationTimer()
	{
		if ( !m_Stopped ) Stop();
	}

	void InstrumentationTimer::Stop()
	{
		auto& instrumentor = Instrumentor::Get();

		auto end_time_point = std::chrono::steady_clock::now();
		auto high_res_start = FloatingPointMicroseconds{m_StartTimePoint.time_since_epoch()};
		auto elapsed_time = std::chrono::time_point_cast<std::chrono::microseconds>(end_time_point).time_since_epoch() -
				std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimePoint).time_since_epoch();

		instrumentor.WriteProfile({m_Name, high_res_start, elapsed_time, std::this_thread::get_id()});

		m_Stopped = true;
	}
}
