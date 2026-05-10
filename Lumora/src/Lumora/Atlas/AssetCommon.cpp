#include "LMPCH.h"

#include "AssetCommon.h"
#include "Lumora/Utilities/Hash.h"

namespace Lumora::Atlas
{
	AssetId AssetId::Generate(std::string_view name)
	{
		uint64_t hash = Hash::FNV1a(name.data(), name.size());
		return AssetId{hash};
	}
}
