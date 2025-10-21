#pragma once

#include "Asset.h"

#include "Lumora/Asset/AssetCommon.h"
#include "Lumora/Asset/AssetProps.h"
#include "Lumora/Common/Base.h"

namespace Lumora
{
	struct AssetTypeInfo
	{
		std::string Name;
		std::string PropsSerializationName;
		std::type_index AssetType{ typeid(Asset) };
		std::type_index AssetPropsType{ typeid(AssetProps) };
		AssetLoadFunction<Asset, AssetProps> LoadFunction;
	};

	// Static class that maps asset types to their properties
	class AssetTypeRegistry
	{
	public:
		static void RegisterType(Ref<AssetTypeInfo> typeInfo);
		static std::string GetName(std::type_index assetType);
		static Ref<AssetTypeInfo> GetTypeInfo(const std::string& type);
		static Ref<AssetTypeInfo> GetTypeInfo(std::type_index assetType);

		template <typename T>
			requires std::is_base_of_v<Asset, T>
		static std::string GetName() { return GetName(typeid(T)); }

		template<typename T>
			requires std::is_base_of_v<Asset, T>
		static Ref<AssetTypeInfo> GetTypeInfo() { return GetTypeInfo(typeid(T)); }
	};

	struct AssetTypeRegistrar
	{
		template <typename T, typename PT>
			requires std::is_base_of_v<Asset, T> and std::is_base_of_v<AssetProps, PT>
		AssetTypeRegistrar(std::string name, AssetLoadFunction<T, PT> loadFunction)
		{
			LM_CORE_ASSERT(loadFunction, "Load function cannot be null")
			LM_CORE_ASSERT(!name.empty(), "Asset type name cannot be empty")

			AssetLoadFunction<Asset, AssetProps> wrapperFunction = [loadFunction](AssetProps& props) -> Ref<Asset>
			{
				return StaticRefCast<Asset>(loadFunction(static_cast<PT&>(props)));
			};

			std::string propsSerializationName = name + "Props";
			LM_REGISTER_FOR_SERIALIZATION_NAMED_VAR(PT, propsSerializationName);

			AssetTypeRegistry::RegisterType(CreateRef<AssetTypeInfo>(
				std::move(name),
				propsSerializationName,
				typeid(T),
				typeid(PT),
				wrapperFunction
			));
		}
	};
}

#define LM_REGISTER_ASSET_TYPE(Name, AssetType, AssetPropsType, LoadFunction)                                                                   \
    class _Registrar##AssetType##__LINE__                                                                                                       \
    {                                                                                                                                           \
        inline static Lumora::AssetTypeRegistrar reg = {Name, static_cast<Lumora::AssetLoadFunction<AssetType, AssetPropsType>>(LoadFunction)}; \
    };

#define LM_REGISTER_ASSET_TYPE_AUTO(AssetType, AssetPropsType, LoadFunction) \
	LM_REGISTER_ASSET_TYPE(#AssetType, AssetType, AssetPropsType, LoadFunction)
