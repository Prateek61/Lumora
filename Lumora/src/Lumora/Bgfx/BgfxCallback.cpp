#include "LMPCH.h"
#include "BgfxCallback.h"

namespace Lumora
{
	void BgfxCallback::fatal(const char* filepath, uint16_t line, bgfx::Fatal::Enum code, const char* str)
	{
		LM_CORE_FATAL("BGFX Fatal Error ({}:{}): {} ({})", filepath, line, str, static_cast<int>(code));
		LM_CORE_ASSERT(code == bgfx::Fatal::DebugCheck, "BGFX Fatal Error")
	}

	void BgfxCallback::traceVargs(const char* filepath, uint16_t line, const char* format, va_list argList)
	{
#ifdef LM_ENABLE_BGFX_LOG
		LM_PROFILE_FUNCTION();

		// Create format args from va_list for fmt
		auto args = fmt::make_format_args(argList);

		// Better approach: use vsnprintf from C standard library
		constexpr size_t bufferSize = 1024;
		char buffer[bufferSize];
		auto _ = vsnprintf(buffer, bufferSize, format, argList);

		std::string formatted(buffer);

		// Remove trailing newlines and carriage returns
		while (!formatted.empty() && (formatted.back() == '\n' || formatted.back() == '\r'))
		{
			formatted.pop_back();
		}

		LM_CORE_TRACE("BGFX Trace ({}:{}): {}", filepath, line, formatted);
#endif
	}


}