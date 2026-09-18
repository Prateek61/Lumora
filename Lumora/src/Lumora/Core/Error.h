#pragma once

#include <cstdint>
#include <string>
#include <source_location>
#include <expected>
#include <string_view>

#pragma warning(push, 0)
#include <spdlog/fmt/fmt.h>
#pragma warning(pop)

#include "Lumora/Core/SmartPointers.h"

namespace Lumora
{
    enum class ErrorCode : uint8_t
    {
        None = 0,
        NotFound,  // Resource not found
        Io,        // IO error
        Invalid,   // Invalid operation
        Parse,     // Error parsing data (e.g. Lua syntax error)
        Script,    // Error in script execution
        TypeMismatch, // Type mismatch error
        NotRegistered, // Type not registered in the system
        NotReady, // Resource not ready for use
        Unknown,   // Umbrella error code for unexpected errors
    };

    constexpr std::string_view ToString(ErrorCode code)
    {
        switch (code)
        {
            case ErrorCode::None: return "None";
            case ErrorCode::NotFound: return "NotFound";
            case ErrorCode::Io: return "Io";
            case ErrorCode::Invalid: return "Invalid";
            case ErrorCode::Parse: return "Parse";
            case ErrorCode::Script: return "Script";
            case ErrorCode::TypeMismatch: return "TypeMismatch";
            case ErrorCode::NotRegistered: return "NotRegistered";
            case ErrorCode::NotReady: return "NotReady";
            case ErrorCode::Unknown: return "Unknown";
        }
        return "Unknown";
    }

    struct ErrorDetail
    {
		std::string Message;
        std::source_location Location;
    };

    struct Error
    {
		ErrorCode Code;
		Scope<ErrorDetail> Detail;

        explicit Error(ErrorCode code, std::string message = "", std::source_location location = std::source_location::current());

        // Overloads
		Error(const Error& other); // Deep copy so Result<T> can be copied
		Error& operator=(const Error& other);
		Error(Error&&) noexcept = default;
		Error& operator=(Error&&) noexcept = default;

        // Puts `context` in front of the message: "loading 'x.lua'" + "expected number" -> "loading 'x.lua': expected number"
		void AddContext(std::string context);
    };

    template <typename T = void>
    using Result = std::expected<T, Error>;

    [[nodiscard]] std::unexpected<Error> MakeError(ErrorCode code, std::string message = {}, std::source_location location = std::source_location::current());

    // On failure, puts a formatted context line in front of the message. On success, nothing is formatted.
    template <typename T, typename... Args>
    [[nodiscard]] Result<T> WithContext(Result<T>&& result, fmt::format_string<Args...> format, Args&&... args)
    {
        if (!result)
			result.error().AddContext(fmt::format(format, std::forward<Args>(args)...));

		return std::move(result);
    }
}

template <>
struct fmt::formatter<Lumora::ErrorCode> : fmt::formatter<std::string_view>
{
    auto format(const Lumora::ErrorCode& code, fmt::format_context& ctx) const
    { 
        return fmt::format_to(ctx.out(), "{}", Lumora::ToString(code));
    }
};
template <>
struct fmt::formatter<Lumora::Error> : fmt::formatter<std::string_view>
{
    auto format(const Lumora::Error& error, fmt::format_context& ctx) const
    {
		if (!error.Detail)
			return fmt::format_to(ctx.out(), "[{}]", Lumora::ToString(error.Code));

		return fmt::format_to(ctx.out(), "{} [{}]", error.Detail->Message, Lumora::ToString(error.Code));
    }
};
