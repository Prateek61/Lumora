#pragma once

#include "Lumora/Asset/AssetCommon.h"

namespace Lumora
{
	// Base class for all assets
	class Asset
	{
	public:
		AssetIdT AssetId = g_INVALID_ASSET_ID;

	public:
		virtual ~Asset() = default;
	};
}