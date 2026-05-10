#pragma once

#include "Lumora/Atlas/AssetCommon.h"
#include "Lumora/Core/SmartPointers.h"

namespace Lumora::Atlas
{
	struct LoaderContext
	{
		std::filesystem::path AssetRoot;
		std::filesystem::path PrimaryPath;
		std::filesystem::path MetaPath;
		AssetMeta Meta;
		Ref<void> Props;
	};

	namespace Detail
	{
		template <typename T, typename Decoded>
		auto DefaultFinalize()
		{
			if constexpr (std::is_same_v<T, Decoded>)
			{
				return [](const LoaderContext&, Decoded&& decoded)
				{
					return std::make_optional<T>(std::move(decoded));
				};
			}
			else
			{
				return nullptr;
			}
		}
	}

	template <typename T, typename Decoded = T>
	struct AssetLoader
	{
		std::string TypeName;
		std::string PropsTypeName;
		std::vector<std::string> FileExtensions;

		std::function<std::optional<Decoded>(const LoaderContext&)> Decode;
		std::function<std::optional<T>(const LoaderContext&, Decoded&&)> Finalize = Detail::DefaultFinalize<T, Decoded>();

		std::function<void(const LoaderContext&, std::vector<std::filesystem::path>&)> CollectSourcePaths;
	};

	struct ErasedLoader
	{
		std::type_index AssetType{typeid(void)};
		std::string TypeName;
		std::string PropsTypeName;
		std::vector<std::string> FileExtensions;

		// Sync-mode entry point, Returns true on success
		std::function<bool(const LoaderContext&, Aether::Entity&)> LoadInto;
		// Forwarded from AssetLoader<T>.CollectSourcePaths. May be empty.
		std::function<void(const LoaderContext&, std::vector<std::filesystem::path>&)> CollectSourcePaths;

		template <typename T, typename Decoded>
		static ErasedLoader From(AssetLoader<T, Decoded> loader)
		{
			ErasedLoader erased;
			erased.AssetType = typeid(T);
			erased.TypeName = std::move(loader.TypeName);
			erased.PropsTypeName = std::move(loader.PropsTypeName);
			erased.FileExtensions = std::move(loader.FileExtensions);
			erased.CollectSourcePaths = std::move(loader.CollectSourcePaths);

			erased.LoadInto = [l = std::move(loader)](const LoaderContext& ctx, Aether::Entity entity) -> bool
			{
				auto decoded = l.Decode(ctx);
				if (!decoded)
					return false;

				auto finalized = l.Finalize(ctx, std::move(*decoded));
				if (!finalized)
					return false;

				entity.Raw().set(std::move(*finalized));
				return true;
			};

			return erased;
		}
	};
}
