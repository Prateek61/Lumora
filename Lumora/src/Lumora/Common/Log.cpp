#include "LMPCH.h"

#include "Log.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace Lumora
{
	Ref<spdlog::logger> Log::s_CoreLogger;
	Ref<spdlog::logger> Log::s_ClientLogger;
	Ref<spdlog::logger> Log::s_LuaLogger;


	void Log::Init(const std::filesystem::path& logFilePath)
	{
		std::vector<spdlog::sink_ptr> log_sinks;
		log_sinks.emplace_back(CreateRef<spdlog::sinks::stdout_color_sink_mt>());
		log_sinks.emplace_back(CreateRef<spdlog::sinks::basic_file_sink_mt>(logFilePath.string(), true));

		log_sinks[0]->set_pattern("%^[%T] %n: %v%$");
		log_sinks[1]->set_pattern("[%T] [%l] %n: %v");

		s_CoreLogger = std::make_shared<spdlog::logger>("ENGINE", begin(log_sinks), end(log_sinks));
		register_logger(s_CoreLogger);
		s_CoreLogger->set_level(spdlog::level::trace);
		s_CoreLogger->flush_on(spdlog::level::trace);

		s_ClientLogger = std::make_shared<spdlog::logger>("APP", begin(log_sinks), end(log_sinks));
		register_logger(s_ClientLogger);
		s_ClientLogger->set_level(spdlog::level::trace);
		s_ClientLogger->flush_on(spdlog::level::trace);

		s_LuaLogger = std::make_shared<spdlog::logger>("LUA", begin(log_sinks), end(log_sinks));
		register_logger(s_LuaLogger);
		s_LuaLogger->set_level(spdlog::level::trace);
		s_LuaLogger->flush_on(spdlog::level::trace);
	}

}
