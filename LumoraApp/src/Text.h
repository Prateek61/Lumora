#pragma once

#include "Lumora.h"

using namespace Lumora;

struct TextProps
{
	std::string Encoding = "utf-8";
};

struct Text
{
	std::string Content;

	static std::optional<Text> FromFile(const std::filesystem::path& path, TextProps props = {});
};

LM_REFLECTABLE(TextProps, Encoding);

std::optional<Text> DecodeText(const Atlas::LoaderContext& context);
Atlas::AssetLoader<Text> MakeTextLoader();
