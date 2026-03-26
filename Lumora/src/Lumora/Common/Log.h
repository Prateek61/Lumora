#pragma once

#include "Lumora/Common/Defines.h"
#include "Lumora/Common/SmartPointers.h"
#include <visit_struct/visit_struct.hpp>
#include "Lumora/Common/Instrumentor.h"

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
			std::string CoreLua = "info";
			std::string CoreBgfx = "info";
			std::string CoreAssets = "info";
			std::string CoreSerializer = "info";
			std::string CoreRenderer = "info";
			std::string Client = "info";
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
		static Ref<spdlog::logger>& GetCoreRendererLogger();

	private:
		static Ref<spdlog::logger> s_CoreLogger;
		static Ref<spdlog::logger> s_ClientLogger;
		static Ref<spdlog::logger> s_CoreLuaLogger;
		static Ref<spdlog::logger> s_CoreBgfxLogger;
		static Ref<spdlog::logger> s_CoreSerializerLogger;
		static Ref<spdlog::logger> s_CoreAssetsLogger;
		static Ref<spdlog::logger> s_CoreRendererLogger;
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
// Renderer Log macros
#define LM_CORE_RENDERER_FATAL(...)     LM_IMPL_LOG_FATAL(CoreRenderer, __VA_ARGS__)
#define LM_CORE_RENDERER_ERROR(...)     LM_IMPL_LOG_ERROR(CoreRenderer, __VA_ARGS__)
#define LM_CORE_RENDERER_WARN(...)      LM_IMPL_LOG_WARN(CoreRenderer, __VA_ARGS__)
#define LM_CORE_RENDERER_INFO(...)      LM_IMPL_LOG_INFO(CoreRenderer, __VA_ARGS__)
#define LM_CORE_RENDERER_TRACE(...)     LM_IMPL_LOG_TRACE(CoreRenderer, __VA_ARGS__)
#define LM_CORE_RENDERER_DEBUG(...)     LM_IMPL_LOG_DEBUG(CoreRenderer, __VA_ARGS__)