#pragma once

#include "Lumora/Core/Defines.h"
#include "Lumora/Core/SmartPointers.h"

#include <filesystem>

// Ignore all warnings from spdlog
#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#pragma warning(pop)

namespace Lumora
{
	namespace Internal
	{
		struct LoggerConfig
		{
			std::string ConsolePattern = "[%T] %^%n: %v%$";
			std::string FilePattern = "[%T] %^[%s:%#] [%l] %n: %v%$";
			std::filesystem::path File = "Lumora.log";
			std::string Core = "info";
			std::string Scripting = "info";
			std::string Client = "info";
		};
	}

	class Log
	{
	public:
		static void Init(const Internal::LoggerConfig& config = {});

		static Ref<spdlog::logger>& GetCoreLogger();
		static Ref<spdlog::logger>& GetClientLogger();
		static Ref<spdlog::logger>& GetScriptLogger();

	private:
		static Ref<spdlog::logger> s_CoreLogger;
		static Ref<spdlog::logger> s_ClientLogger;
		static Ref<spdlog::logger> s_ScriptLogger;
	};
}

/// Logging Macros
/// Log levels: 0 = Off, 1 = Fatal, 2 = Error, 3 = Warn, 4 = Info, 5 = Trace, 6 = Debug

#define LM_IMPL_SPDLOG_SOURCE_LOC() spdlog::source_loc(__FILE__, __LINE__, Lumora::InstrumentorUtils::CleanupOutputString(LM_FUNC_SIG, "__cdecl ").Data)

#if LM_LOG_LEVEL >= 1
#define LM_IMPL_LOG_FATAL(Type, ...)   ::Lumora::Log::Get##Type##Logger()->log(LM_IMPL_SPDLOG_SOURCE_LOC(), spdlog::level::critical, __VA_ARGS__)
#else
#define LM_IMPL_LOG_FATAL(Type, ...)
#endif
#if LM_LOG_LEVEL >= 2
#define LM_IMPL_LOG_ERROR(Type, ...)   ::Lumora::Log::Get##Type##Logger()->log(LM_IMPL_SPDLOG_SOURCE_LOC(), spdlog::level::err, __VA_ARGS__)
#else
#define LM_IMPL_LOG_ERROR(Type, ...)
#endif
#if LM_LOG_LEVEL >= 3
#define LM_IMPL_LOG_WARN(Type, ...)    ::Lumora::Log::Get##Type##Logger()->log(LM_IMPL_SPDLOG_SOURCE_LOC(), spdlog::level::warn, __VA_ARGS__)
#else
#define LM_IMPL_LOG_WARN(Type, ...)
#endif
#if LM_LOG_LEVEL >= 4
#define LM_IMPL_LOG_INFO(Type, ...)    ::Lumora::Log::Get##Type##Logger()->log(LM_IMPL_SPDLOG_SOURCE_LOC(), spdlog::level::info, __VA_ARGS__)
#else
#define LM_IMPL_LOG_INFO(Type, ...)
#endif
#if LM_LOG_LEVEL >= 5
#define LM_IMPL_LOG_TRACE(Type, ...)   ::Lumora::Log::Get##Type##Logger()->log(LM_IMPL_SPDLOG_SOURCE_LOC(), spdlog::level::trace, __VA_ARGS__)
#else
#define LM_IMPL_LOG_TRACE(Type, ...)
#endif
#if LM_LOG_LEVEL >= 6
#define LM_IMPL_LOG_DEBUG(Type, ...)   ::Lumora::Log::Get##Type##Logger()->log(LM_IMPL_SPDLOG_SOURCE_LOC(), spdlog::level::debug, __VA_ARGS__)
#else
#define LM_IMPL_LOG_DEBUG(Type, ...)
#endif

// Core log macros
#define LM_CORE_FATAL(...)     LM_IMPL_LOG_FATAL(Core, __VA_ARGS__)
#define LM_CORE_ERROR(...)     LM_IMPL_LOG_ERROR(Core, __VA_ARGS__)
#define LM_CORE_WARN(...)      LM_IMPL_LOG_WARN(Core, __VA_ARGS__)
#define LM_CORE_INFO(...)      LM_IMPL_LOG_INFO(Core, __VA_ARGS__)
#define LM_CORE_TRACE(...)     LM_IMPL_LOG_TRACE(Core, __VA_ARGS__)
#define LM_CORE_DEBUG(...)     LM_IMPL_LOG_DEBUG(Core, __VA_ARGS__)
// Client log macros
#define LM_LOG_FATAL(...)          LM_IMPL_LOG_FATAL(Client, __VA_ARGS__)
#define LM_LOG_ERROR(...)          LM_IMPL_LOG_ERROR(Client, __VA_ARGS__)
#define LM_LOG_WARN(...)           LM_IMPL_LOG_WARN(Client, __VA_ARGS__)
#define LM_LOG_INFO(...)           LM_IMPL_LOG_INFO(Client, __VA_ARGS__)
#define LM_LOG_TRACE(...)          LM_IMPL_LOG_TRACE(Client, __VA_ARGS__)
#define LM_LOG_DEBUG(...)          LM_IMPL_LOG_DEBUG(Client, __VA_ARGS__)