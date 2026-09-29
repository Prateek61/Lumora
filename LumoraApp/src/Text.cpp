#include "Lumora.h"

#include "Text.h"
#include <utility>

std::optional<Text> Text::FromFile(const std::filesystem::path& path, TextProps props)
{
	LM_PROFILE_FUNCTION();
	
	std::ifstream in(path, std::ios::in | std::ios::binary);
	if (!in)
	{
		LM_LOG_ERROR("Failed to open text file '{}'", path.string());
		return std::nullopt;
	}

	std::stringstream buffer;
	buffer << in.rdbuf();

	Text res;
	res.Content = buffer.str();
	return std::move(res);
}

std::optional<Text> DecodeText(const Atlas::LoaderContext& context)
{
	LM_PROFILE_FUNCTION();
	
	const auto* props = static_cast<const TextProps*>(context.Props.get());
	auto loaded = Text::FromFile(context.AssetRoot / context.PrimaryPath, props ? *props : TextProps{});
	if (!loaded)
	{
		LM_LOG_ERROR("DecodeText: Failed to load text from '{}'", context.PrimaryPath.string());
	}
	
	return loaded;
}

Atlas::AssetLoader<Text> MakeTextLoader()
{
	LM_PROFILE_FUNCTION();
	
	Atlas::AssetLoader<Text> loader;
	loader.TypeName = "Text";
	loader.SetPropsType<TextProps>("TextProps");
	loader.FileExtensions = {".txt"};
	loader.Decode = DecodeText;
	return loader;
}
