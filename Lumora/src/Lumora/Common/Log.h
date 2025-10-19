#pragma once

#include "Lumora/Common/Defines.h"
#include "Lumora/Common/SmartPointers.h"

#include <filesystem>

// Ignore all warnings from spdlog
#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#pragma warning(pop)

namespace Lumora
{
	class Log
	{
	public:
		static void Init(const std::filesystem::path& logFilePath = "Lumora.log");

		static Ref<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		static Ref<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }
		static Ref<spdlog::logger>& GetLuaLogger() { return s_LuaLogger; }
		static Ref<spdlog::logger>& GetBgfxLogger() { return s_BgfxLogger; }

	private:
		static Ref<spdlog::logger> s_CoreLogger;
		static Ref<spdlog::logger> s_ClientLogger;
		static Ref<spdlog::logger> s_LuaLogger;
		static Ref<spdlog::logger> s_BgfxLogger;
	};
}

// Core log macros
#ifdef LM_ENABLE_CORE_LOG

#define LM_CORE_FATAL(...)   ::Lumora::Log::GetCoreLogger()->critical(__VA_ARGS__)
#define LM_CORE_ERROR(...)   ::Lumora::Log::GetCoreLogger()->error(__VA_ARGS__)
#define LM_CORE_WARN(...)    ::Lumora::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define LM_CORE_INFO(...)    ::Lumora::Log::GetCoreLogger()->info(__VA_ARGS__)
#define LM_CORE_TRACE(...)   ::Lumora::Log::GetCoreLogger()->trace(__VA_ARGS__)

#else

#define LM_CORE_FATAL(...)
#define LM_CORE_ERROR(...)
#define LM_CORE__WARN(...)
#define LM_CORE__INFO(...)
#define LM_CORE_TRACE(...)

#endif

// Client log macros
#ifdef LM_ENABLE_CLIENT_LOG
#define LM_FATAL(...)        ::Lumora::Log::GetClientLogger()->critical(__VA_ARGS__)
#define LM_ERROR(...)        ::Lumora::Log::GetClientLogger()->error(__VA_ARGS__)
#define LM_WARN(...)         ::Lumora::Log::GetClientLogger()->warn(__VA_ARGS__)
#define LM_INFO(...)         ::Lumora::Log::GetClientLogger()->info(__VA_ARGS__)
#define LM_TRACE(...)        ::Lumora::Log::GetClientLogger()->trace(__VA_ARGS__)
#else
#define LM_FATAL(...)
#define LM_ERROR(...)
#define LM_WARN(...)
#define LM_INFO(...)
#define LM_TRACE(...)
#endif

#define LM_ENABLE_SERIALIZER_LOG

#ifdef LM_ENABLE_SERIALIZER_LOG
#define LM_CORE_SERIALIZER_FATAL(...)  LM_CORE_FATAL(__VA_ARGS__)
#define LM_CORE_SERIALIZER_ERROR(...)  LM_CORE_ERROR(__VA_ARGS__)
#define LM_CORE_SERIALIZER_WARN(...)   LM_CORE_WARN(__VA_ARGS__)
#define LM_CORE_SERIALIZER_INFO(...)   LM_CORE_INFO(__VA_ARGS__)
#define LM_CORE_SERIALIZER_TRACE(...)  LM_CORE_TRACE(__VA_ARGS__)
#else
#define LM_CORE_SERIALIZER_FATAL(...)
#define LM_CORE_SERIALIZER_ERROR(...)
#define LM_CORE_SERIALIZER_WARN(...)
#define LM_CORE_SERIALIZER_INFO(...)
#define LM_CORE_SERIALIZER_TRACE(...)
#endif