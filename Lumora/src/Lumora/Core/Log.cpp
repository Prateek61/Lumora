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

	// Pads the string with spaces at the back to reach the total length
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
	Ref<spdlog::logger> Log::s_ScriptLogger;

	void Log::Init(const Internal::LoggerConfig& config)
	{
		LM_PROFILE_FUNCTION();

		if (LogInitialized)
		{
			LM_CORE_WARN("Log system already initialized! Ignoring duplicate initialization.");
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
		s_ScriptLogger = create_logger(config.Scripting, PadString("SCRIPT", 10));
		LogInitialized = true;
	}

	bool Log::IsInitialized()
	{
		return LogInitialized;
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
	Ref<spdlog::logger>& Log::GetScriptLogger()
	{
		LM_CORE_ASSERT(s_ScriptLogger, "Lua Logger not initialized!")
		return s_ScriptLogger;
	}
}
