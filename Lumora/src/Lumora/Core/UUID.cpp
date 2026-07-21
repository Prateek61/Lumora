#include "LMPCH.h"

#include "UUID.h"

#include <atomic>

namespace
{
	std::atomic<uint64_t> s_CurrentUUID = 0;
}

namespace Lumora
{
	UUID UUID::Generate()
	{
		return { ++s_CurrentUUID };
	}
}
