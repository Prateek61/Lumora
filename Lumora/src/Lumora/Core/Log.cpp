#include "LMPCH.h"

#include "Log.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace
{
	spdlog::level::level_enum StringToLevel(const std::string& levelStr)
	{
		if (levelStr == "trace") return spdlog::level::trace;
		if (levelStr == "debug") return spdlog::level::debug;
		if (levelStr == "info") return spdlog::level::info;
		if (levelStr == "warn") return spdlog::level::warn;
		if (levelStr == "error") return spdlog::level::err;
		if (levelStr == "fatal") return spdlog::level::critical;
		if (levelStr == "off") return spdlog::level::off;
		return spdlog::level::info; // Default level
	}

	Lumora::Ref<spdlog::logger>& GetDefaultLogger()
	{
		static Lumora::Ref<spdlog::logger> default_logger = spdlog::default_logger();
		return default_logger;
	}

	// Pads the string with spaces in the front to reach the total length
	std::string PadString(const std::string& str, size_t totalLength)
	{
		if (str.length() >= totalLength)
			return str;
		return str + std::string(totalLength - str.length(), ' ');
	}

	bool LogInitialized = false;
}

namespace Lumora
{
	Ref<spdlog::logger> Log::s_CoreLogger;
	Ref<spdlog::logger> Log::s_ClientLogger;
	Ref<spdlog::logger> Log::s_CoreLuaLogger;
	Ref<spdlog::logger> Log::s_CoreBgfxLogger;
	Ref<spdlog::logger> Log::s_CoreSerializerLogger;
	Ref<spdlog::logger> Log::s_CoreAssetsLogger;
	Ref<spdlog::logger> Log::s_CoreRendererLogger;

	void Log::Init(const Internal::LoggerConfig& config)
	{
		LM_PROFILE_FUNCTION();

		if (LogInitialized)
		{
			return;
		}

		std::vector<spdlog::sink_ptr> log_sinks;
		log_sinks.emplace_back(CreateRef<spdlog::sinks::stdout_color_sink_mt>());
		log_sinks.emplace_back(CreateRef<spdlog::sinks::basic_file_sink_mt>(config.File.string()));

		log_sinks[0]->set_pattern(config.ConsolePattern);
		log_sinks[1]->set_pattern(config.FilePattern);

		auto create_logger = [&](const std::string& levelStr, const std::string& name)
		{
			auto begin = std::begin(log_sinks);
			auto end = std::end(log_sinks);
			auto logger = std::make_shared<spdlog::logger>(name, begin, end);
			register_logger(logger);
			auto level = StringToLevel(levelStr);
			logger->set_level(level);
			logger->flush_on(level);
			return logger;
		};

		s_CoreLogger = create_logger(config.Core, PadString("ENGINE", 10));
		s_ClientLogger = create_logger(config.Client, PadString("APP", 10));
		s_CoreBgfxLogger = create_logger(config.CoreBgfx, PadString("BGFX", 10));
		s_CoreLuaLogger = create_logger(config.CoreLua, PadString("LUA", 10));
		s_CoreSerializerLogger = create_logger(config.CoreSerializer, PadString("SERIALIZER", 10));
		s_CoreAssetsLogger = create_logger(config.CoreAssets, PadString("ASSETS", 10));
		s_CoreRendererLogger = create_logger(config.CoreRenderer, PadString("RENDERER", 10));
		LogInitialized = true;
	}

	Ref<spdlog::logger>& Log::GetCoreLogger()
	{
		LM_CORE_ASSERT(s_CoreLogger, "Core Logger not initialized!")
		return s_CoreLogger;
	}
	Ref<spdlog::logger>& Log::GetClientLogger()
	{
		LM_CORE_ASSERT(s_ClientLogger, "Client Logger not initialized!")
		return s_ClientLogger;
	}
	Ref<spdlog::logger>& Log::GetLuaLogger()
	{
		LM_CORE_ASSERT(s_CoreLuaLogger, "Lua Logger not initialized!")
		return s_CoreLuaLogger;
	}
	Ref<spdlog::logger>& Log::GetBgfxLogger()
	{
		LM_CORE_ASSERT(s_CoreBgfxLogger, "Bgfx Logger not initialized!")
		return s_CoreBgfxLogger;
	}
	Ref<spdlog::logger>& Log::GetCoreSerializerLogger()
	{
		LM_CORE_ASSERT(s_CoreSerializerLogger, "Serializer Logger not initialized!")
		return s_CoreSerializerLogger;
	}
	Ref<spdlog::logger>& Log::GetCoreAssetsLogger()
	{
		LM_CORE_ASSERT(s_CoreAssetsLogger, "Assets Logger not initialized!")
		return s_CoreAssetsLogger;
	}
	Ref<spdlog::logger>& Log::GetCoreRendererLogger()
	{
		LM_CORE_ASSERT(s_CoreRendererLogger, "Renderer Logger not initialized!")
		return s_CoreRendererLogger;
	}
}
