#include "LMPCH.h"

#include "UUID.h"

namespace
{
	uint64_t s_CurrentUUID = 0;
}

namespace Lumora
{
	UUID UUID::Generate()
	{
		return { ++s_CurrentUUID };
	}
}
