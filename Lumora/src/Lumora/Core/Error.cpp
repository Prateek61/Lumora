#include "LMPCH.h"
#include "Error.h"

namespace Lumora
{
	Error::Error(ErrorCode code, std::string message, std::source_location location) : Code(code), Detail(CreateScope<ErrorDetail>(std::move(message), location))
	{}

	Error::Error(const Error& other) : Code(other.Code),
	      Detail(other.Detail ? CreateScope<ErrorDetail>(*other.Detail) : nullptr)
	{}

	Error& Error::operator=(const Error& other)
	{
		if (this != &other)
			*this = Error(other);

		return *this;
	}

	void Error::AddContext(std::string context)
	{
		if (!Detail)
			Detail = CreateScope<ErrorDetail>();

		if (Detail->Message.empty())
			Detail->Message = std::move(context);
		else
			Detail->Message.insert(0, ": ").insert(0, context);
	}

	template <typename T, typename... Args>
	[[nodiscard]] Result<T> WithContext(Result<T>&& result, fmt::format_string<Args...> format, Args&&... args)
	{
		if (!result)
			result.error().AddContext(fmt::format(format, std::forward<Args>(args)...));

		return std::move(result);
	}

	std::unexpected<Error> MakeError(ErrorCode code, std::string message, std::source_location location)
	{ 
		return std::unexpected<Error>(Error(code, std::move(message), location));
	}
}
