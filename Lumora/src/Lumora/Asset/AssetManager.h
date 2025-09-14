#pragma once

#include "Lumora/Common/Base.h"
#include "Lumora/Asset/Asset.h"
#include "Lumora/Asset/AssetProps.h"
#include "Lumora/Asset/AssetRecord.h"
#include "Lumora/Asset/AssetHandle.h"
#include "Lumora/Asset/AssetRegistry.h"
#include "Lumora/Asset/AssetStorage.h"

namespace Lumora
{
	class AssetManager
	{
	public:
	private:
		AssetStorage m_Storage;
		AssetRegistry m_Registry;
	};
}