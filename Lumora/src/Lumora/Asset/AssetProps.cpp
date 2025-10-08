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

		auto type = serializer.DeserializeFromFile<JustAssetType>(propsPath).Type;
		auto type_info = AssetTypeRegistry::GetTypeInfo(type);
		Ref<AssetProps> props = StaticRefCast<AssetProps>(serializer.DeserializeFromFile(type_info.PropsSerializationName, propsPath));
		return props;
	}


}