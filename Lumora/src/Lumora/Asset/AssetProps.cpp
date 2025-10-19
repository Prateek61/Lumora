#include "LMPCH.h"
#include "AssetProps.h"

#include "Lumora/Scripting/LuaSerializer.h"
#include "Lumora/Asset/AssetTypeRegistry.h"
#include "visit_struct/visit_struct.hpp"

struct JustAssetType
{
	std::string Type;
};
VISITABLE_STRUCT(JustAssetType, Type);

namespace Lumora
{
	std::string AssetProps::ToString() const
	{
		LM_PROFILE_FUNCTION();
		std::stringstream ss;
		ss << "AssetProps { Name: " << Name << ", Type: " << Type << ", HotReload: " << (HotReload ? "true" : "false");
		ss <<  ", AssetId: " << AssetId;
		return ss.str();
	}

	Ref<AssetProps> AssetProps::DeSerialize(const std::filesystem::path& propsPath, LuaSerializer& serializer)
	{
		LM_PROFILE_FUNCTION();

		std::string type;
		try
		{
			type = serializer.DeserializeFromFile<JustAssetType>(propsPath).Type;
		} catch (const Lua::LuaError& err)
		{
			LM_CORE_ASSETS_ERROR("Failed to deserialize AssetProps: {}", err.what());
		}

		if (type.empty())
		{
			LM_CORE_ASSETS_ERROR("Failed to deserialize AssetProps. 'Type' field is missing or empty in file: {}", propsPath.string());
			return nullptr;
		}

		auto type_info = AssetTypeRegistry::GetTypeInfo(type);

		if (!type_info)
		{
			LM_CORE_ASSETS_ERROR("Failed to deserialize AssetProps. Asset type not registered: {} in file: {}", type, propsPath.string());
			return nullptr;
		}

		Ref<AssetProps> props = StaticRefCast<AssetProps>(serializer.DeserializeFromFile(type_info->PropsSerializationName, propsPath));
		return props;
	}


}