#pragma once

#include "Lumora/Asset/AssetCommon.h"
#include "Lumora/Asset/AssetTypeRegistry.h"

namespace Lumora
{
    class AssetLoader
    {
    public:
        static Ref<Asset> Load(AssetProps& props);
    };
}