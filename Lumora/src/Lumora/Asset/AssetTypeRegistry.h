#pragma once

#include "Lumora/Asset/AssetCommon.h"
#include "Lumora/Asset/AssetProps.h"
#include "Lumora/Common/Base.h"

namespace Lumora
{
	struct AssetTypeInfo
	{
		std::string Name;
		std::string PropsSerializationName;
		std::type_index AssetType;
		std::type_index AssetPropsType;
		AssetLoadFunction<Asset, AssetProps> LoadFunction;
	};

	// Static class that maps asset types to their properties
	class AssetTypeRegistry
	{
	public:
		static void RegisterType(AssetTypeInfo typeInfo);
		static std::type_index GetCppType(const std::string& type);
		static AssetTypeInfo* GetTypeInfo(const std::string& type);
		static AssetTypeInfo* GetTypeInfo(std::type_index cppType);
	};

	struct AssetTypeRegistrar
	{
		template <typename T, typename PT>
			requires std::is_base_of_v<Asset, T> and std::is_base_of_v<AssetProps, PT>
		AssetTypeRegistrar(const std::string& name, AssetLoadFunction<T, PT> loadFunction)
		{
			LM_CORE_ASSERT(loadFunction, "Load function cannot be null");
			LM_CORE_ASSERT(!name.empty(), "Asset type name cannot be empty");

			AssetLoadFunction<Asset, AssetProps> wrapperFunction = [loadFunction](AssetProps& props) -> Ref<Asset>
			{
				return StaticRefCast<Asset>(loadFunction(static_cast<PT&>(props)));
			};

			LM_REGISTER_FOR_SERIALIZATION_NAMED(PT, name + "Props");

			AssetTypeRegistry::RegisterType({
				.Name = name,
				.PropsSerializationName = name + "Props",
				.AssetPropsType = typeid(PT),
				.AssetType = typeid(T),
				.LoadFunction = wrapperFunction
			});
		}
	};
}
