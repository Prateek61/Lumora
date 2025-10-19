#pragma once

#include "Lumora/Common/Defines.h"
#include "Lumora/Common/SmartPointers.h"
#include <visit_struct/visit_struct.hpp>

#include <filesystem>

// Ignore all warnings from spdlog
#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#pragma warning(pop)

namespace Lumora
{
	namespace Internal
	{
		struct SingleLoggerConfig
		{
			std::string Level = "info";
			bool EnableConsoleLog = true;
			bool EnableFileLog = true;
		};

		struct LoggerConfig
		{
			std::string ConsolePattern = "[%T] %^%n: %v%$";
			std::string FilePattern = "[%T] %^[%s:%#] [%l] %n: %v%$";
			std::filesystem::path File = "Lumora.log";
			SingleLoggerConfig Core;
			SingleLoggerConfig CoreLua;
			SingleLoggerConfig CoreBgfx;
			SingleLoggerConfig CoreAssets;
			SingleLoggerConfig CoreSerializer;
			SingleLoggerConfig Client;
		};
	}

	class Log
	{
	public:
		static void Init(const Internal::LoggerConfig& config = {});

		static Ref<spdlog::logger>& GetCoreLogger();
		static Ref<spdlog::logger>& GetClientLogger();
		static Ref<spdlog::logger>& GetLuaLogger();
		static Ref<spdlog::logger>& GetBgfxLogger();
		static Ref<spdlog::logger>& GetCoreSerializerLogger();
		static Ref<spdlog::logger>& GetCoreAssetsLogger();

	private:
		static Ref<spdlog::logger> s_CoreLogger;
		static Ref<spdlog::logger> s_ClientLogger;
		static Ref<spdlog::logger> s_CoreLuaLogger;
		static Ref<spdlog::logger> s_CoreBgfxLogger;
		static Ref<spdlog::logger> s_CoreSerializerLogger;
		static Ref<spdlog::logger> s_CoreAssetsLogger;
	};
}

VISITABLE_STRUCT(Lumora::Internal::SingleLoggerConfig, Level, EnableConsoleLog, EnableFileLog);
VISITABLE_STRUCT(Lumora::Internal::LoggerConfig, ConsolePattern, FilePattern, File ,Core, CoreLua, CoreBgfx, CoreAssets, CoreSerializer, Client);


/// Logging Macros
/// Log levels: 0 = Off, 1 = Fatal, 2 = Error, 3 = Warn, 4 = Info, 5 = Trace, 6 = Debug

#define LM_IMPL_SPDLOG_SOURCE_LOC() spdlog::source_loc(__FILE__, __LINE__, "")

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
#define LM_FATAL(...)          LM_IMPL_LOG_FATAL(Client, __VA_ARGS__)
#define LM_ERROR(...)          LM_IMPL_LOG_ERROR(Client, __VA_ARGS__)
#define LM_WARN(...)           LM_IMPL_LOG_WARN(Client, __VA_ARGS__)
#define LM_INFO(...)           LM_IMPL_LOG_INFO(Client, __VA_ARGS__)
#define LM_TRACE(...)          LM_IMPL_LOG_TRACE(Client, __VA_ARGS__)
#define LM_DEBUG(...)          LM_IMPL_LOG_DEBUG(Client, __VA_ARGS__)
// Serializer Log macros
#define LM_CORE_SERIALIZER_FATAL(...)     LM_IMPL_LOG_FATAL(CoreSerializer, __VA_ARGS__)
#define LM_CORE_SERIALIZER_ERROR(...)     LM_IMPL_LOG_ERROR(CoreSerializer, __VA_ARGS__)
#define LM_CORE_SERIALIZER_WARN(...)      LM_IMPL_LOG_WARN(CoreSerializer, __VA_ARGS__)
#define LM_CORE_SERIALIZER_INFO(...)      LM_IMPL_LOG_INFO(CoreSerializer, __VA_ARGS__)
#define LM_CORE_SERIALIZER_TRACE(...)     LM_IMPL_LOG_TRACE(CoreSerializer, __VA_ARGS__)
#define LM_CORE_SERIALIZER_DEBUG(...)     LM_IMPL_LOG_DEBUG(CoreSerializer, __VA_ARGS__)
// Assets Log macros
#define LM_CORE_ASSETS_FATAL(...)     LM_IMPL_LOG_FATAL(CoreAssets, __VA_ARGS__)
#define LM_CORE_ASSETS_ERROR(...)     LM_IMPL_LOG_ERROR(CoreAssets, __VA_ARGS__)
#define LM_CORE_ASSETS_WARN(...)      LM_IMPL_LOG_WARN(CoreAssets, __VA_ARGS__)
#define LM_CORE_ASSETS_INFO(...)      LM_IMPL_LOG_INFO(CoreAssets, __VA_ARGS__)
#define LM_CORE_ASSETS_TRACE(...)     LM_IMPL_LOG_TRACE(CoreAssets, __VA_ARGS__)
#define LM_CORE_ASSETS_DEBUG(...)     LM_IMPL_LOG_DEBUG(CoreAssets, __VA_ARGS__)