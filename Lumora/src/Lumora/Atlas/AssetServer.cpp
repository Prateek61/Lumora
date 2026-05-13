#include "LMPCH.h"

#include "AssetServer.h"
#include "Lumora/Rune/LuaSerializer.h"

namespace Lumora::Atlas
{
	AssetServer::AssetServer(Aether::World& world, Rune::LuaSerializer& serializer) : m_World(world), m_Serializer(serializer)
	{
		LM_PROFILE_FUNCTION();
		m_AssetsRootEntity = m_World.CreateEntity("Assets");
	}

	AssetServer::~AssetServer()
	{
		LM_PROFILE_FUNCTION();

		if (m_AssetsRootEntity.IsValid())
		{
			m_AssetsRootEntity.Destruct();
		}
	}

	UntypedAssetHandle AssetServer::GetUntyped(std::string_view name) const
	{
		LM_PROFILE_FUNCTION();

		auto lock = ReadLock(m_AssetRegistryMutex);

		auto name_it = m_AssetIdsByName.find(std::string(name));
		if (name_it == m_AssetIdsByName.end())
			return {};

		auto entry_it = m_AssetsById.find(name_it->second);
		LM_CORE_ASSERT(entry_it != m_AssetsById.end(), "Asset found in m_AssetIdsByName but missing from m_AssetsById");

		const auto& entry = entry_it->second;
		UntypedAssetHandle handle;
		handle.Id = entry.Id;
		handle.Entity = entry.Entity;
		handle.Type = entry.Loader ? entry.Loader->AssetType : std::type_index(typeid(void));
		return handle;
	}

	Aether::Entity AssetServer::Resolve(AssetId id) const
	{
		LM_PROFILE_FUNCTION();

		auto lock = ReadLock(m_AssetRegistryMutex);

		auto it = m_AssetsById.find(id);
		if (it == m_AssetsById.end())
			return Aether::Entity{};

		return it->second.Entity;
	}

	Aether::Entity AssetServer::Resolve(std::string_view name) const
	{
		LM_PROFILE_FUNCTION();

		AssetId id;
		{
			auto lock = ReadLock(m_AssetRegistryMutex);
			auto it = m_AssetIdsByName.find(std::string(name));
			if (it == m_AssetIdsByName.end())
				return Aether::Entity{};
			id = it->second;
		}

		return Resolve(id);
	}

	bool AssetServer::Unload(AssetId id)
	{
		LM_PROFILE_FUNCTION();

		Aether::Entity entity;
		{
			auto lock = ReadLock(m_AssetRegistryMutex);
			auto it = m_AssetsById.find(id);
			if (it == m_AssetsById.end())
				return false;
			entity = it->second.Entity;
		}

		if (entity.IsValid())
		{
			entity.Destruct();
		}

		auto lock = WriteLock(m_AssetRegistryMutex);

		auto it = m_AssetsById.find(id);
		if (it == m_AssetsById.end())
			return true;

		// Remove name -> id mapping
		for (auto nit = m_AssetIdsByName.begin(); nit != m_AssetIdsByName.end();)
		{
			if (nit->second == id) nit = m_AssetIdsByName.erase(nit);
			else                   ++nit;
		}

		for (auto pit = m_PrimaryByPath.begin(); pit != m_PrimaryByPath.end();)
		{
			if (pit->second == id) pit = m_PrimaryByPath.erase(pit);
			else                   ++pit;
		}

		for (auto pit = m_PathSubscribers.begin(); pit != m_PathSubscribers.end();)
		{
			auto& vec = pit->second;
			vec.erase(std::remove(vec.begin(), vec.end(), id), vec.end());
			if (vec.empty()) pit = m_PathSubscribers.erase(pit);
			else             ++pit;
		}

		for (auto dit = m_DefaultAssets.begin(); dit != m_DefaultAssets.end();)
		{
			if (dit->second == id) dit = m_DefaultAssets.erase(dit);
			else                   ++dit;
		}

		m_AssetsById.erase(it);
		return true;
	}

	void AssetServer::Scan(const std::filesystem::path& assetRoot)
	{
		LM_PROFILE_FUNCTION();

		std::error_code ec;
		m_AssetRoot = std::filesystem::canonical(assetRoot, ec);
		if (ec)
		{
			LM_CORE_WARN("AssetServer::Scan: failed to canonicalise '{}': {}; falling back to raw path", assetRoot.string(), ec.message());
			m_AssetRoot = assetRoot;
		}

		if (!std::filesystem::exists(m_AssetRoot) || !std::filesystem::is_directory(m_AssetRoot))
		{
			LM_CORE_ERROR("AssetServer::Scan: asset root '{}' does not exist or is not a directory", m_AssetRoot.string());
			return;
		}

		LM_CORE_INFO("Scanning asset root '{}'", m_AssetRoot.string());

		namespace fs = std::filesystem;

		// Pass 1: Standalone .asset.lua files 
		for (const auto& entry : fs::recursive_directory_iterator(m_AssetRoot))
		{
			if (!entry.is_regular_file())
				continue;
			const auto& p = entry.path();
			if (!IsAssetLuaFile(p))
				continue;

			auto primary = StripAssetLuaSuffix(p);
			if (fs::exists(primary))
				continue;

			RegisterFromMeta(p);
		}

		// Pass 2: Regular files with optional sidecar
		for (auto& entry : fs::recursive_directory_iterator(m_AssetRoot))
		{
			if (!entry.is_regular_file())
				continue;
			const auto& p = entry.path();
			if (IsAssetLuaFile(p))
				continue;

			auto ext = p.extension().string();

			bool has_loader;
			{
				auto lock = ReadLock(m_TypeRegistryMutex);
				has_loader = m_TypeNameByExtension.contains(ext);
			}

			if (!has_loader)
			{
				LM_CORE_DEBUG("No loader registered for extension '{}'; skipping asset '{}'", ext, p.string());
				continue;
			}

			RegisterSingleFile(p);
		}

		// Pass 3: Eager load
		std::vector<AssetId> to_load;
		{
			auto lock = ReadLock(m_AssetRegistryMutex);
			to_load.reserve(m_AssetsById.size());
			for (const auto& [id, entry] : m_AssetsById)
			{
				if (entry.Loader)
				{
					to_load.push_back(id);
				}
			}
		}

		for (auto id : to_load)
		{
			AssetEntry snapshot;
			{
				auto lock = ReadLock(m_AssetRegistryMutex);
				auto it = m_AssetsById.find(id);
				if (it == m_AssetsById.end())
				{
					LM_CORE_ERROR("Asset with id {} disappeared between scan and load; skipping", id.Id);
					continue;
				}
				snapshot = it->second;
			}

			RunLoad(snapshot);

			{
				auto lock = WriteLock(m_AssetRegistryMutex);
				auto it = m_AssetsById.find(id);
				if (it != m_AssetsById.end())
					it->second.PropsBlob = std::move(snapshot.PropsBlob);
			}
		}

		LM_CORE_INFO("AssetServer: scan complete - {} assets loaded", to_load.size());
	}

	void AssetServer::QueueReload(AssetId id)
	{
		LM_PROFILE_FUNCTION();

		auto lock = WriteLock(m_ReloadQueueMutex);
		m_ReloadQueue.push_back(id);
	}

	void AssetServer::Pump()
	{
		LM_PROFILE_FUNCTION();

		std::vector<AssetId> queued;
		{
			auto lock = WriteLock(m_ReloadQueueMutex);
			queued.swap(m_ReloadQueue);
		}
		if (queued.empty())
			return;

		for (auto id : queued)
		{
			AssetEntry snapshot;
			{
				auto lock = ReadLock(m_AssetRegistryMutex);
				auto it = m_AssetsById.find(id);
				if (it == m_AssetsById.end())
				{
					LM_CORE_ERROR("Asset with id {} disappeared before reload; skipping", id.Id);
					continue;
				}
				if (!it->second.Loader)
				{
					LM_CORE_WARN("Asset with id {} has no loader; cannot reload", id.Id);
					continue;
				}
				if (!it->second.Entity.IsValid())
				{
					LM_CORE_ERROR("Asset with id {} has invalid entity; cannot reload", id.Id);
					continue;
				}
				snapshot = it->second;
			}

			RunLoad(snapshot);

			{
				auto lock = WriteLock(m_AssetRegistryMutex);
				auto it = m_AssetsById.find(id);
				if (it != m_AssetsById.end())
					it->second.PropsBlob = std::move(snapshot.PropsBlob);
			}
		}
	}

	std::vector<AssetId> AssetServer::LookupByPath(const std::filesystem::path& canonicalPath) const
	{
		LM_PROFILE_FUNCTION();
		
		auto lock = ReadLock(m_AssetRegistryMutex);
		auto it = m_PathSubscribers.find(canonicalPath.string());
		if (it == m_PathSubscribers.end())
			return {};
		return it->second;
	}

	Aether::Entity AssetServer::CreateAssetEntity(const AssetMeta& meta)
	{
		LM_PROFILE_FUNCTION();

		Aether::Entity entity = m_World.CreateEntity()
									   .ChildOf(m_AssetsRootEntity)
									   .Add<AssetTag>()
			                           .Set<AssetMeta>(meta);

		return entity;
	}

	void AssetServer::RegisterSingleFile(const std::filesystem::path& assetFile_in)
	{
		LM_PROFILE_FUNCTION();

		std::error_code ec;
		auto assetFile = std::filesystem::weakly_canonical(assetFile_in, ec);
		if (ec) assetFile = assetFile_in;

		// Find a loader claiming this file's extension
		auto ext = assetFile.extension().string();
		ErasedLoader* loader;
		{
			auto lock = ReadLock(m_TypeRegistryMutex);

			auto type_it = m_TypeNameByExtension.find(ext);
			if (type_it == m_TypeNameByExtension.end())
			{
				LM_CORE_WARN("No loader registered for extension '{}'; skipping asset '{}'", ext, assetFile.string());
				return;
			}

			auto loader_it = m_LoadersByType.find(type_it->second);
			if (loader_it == m_LoadersByType.end())
			{
				LM_CORE_ERROR("Inconsistent state: no loader found for type '{}' claimed by extension '{}'", type_it->second, ext);
				return;
			}
			loader = &loader_it->second;
		}

		// Look for sidecar
		auto sidecar_path = assetFile;
		sidecar_path += ".asset.lua";
		bool sidecar_exists = std::filesystem::exists(sidecar_path);

		// AssetMetaFile (Name, Type)
		AssetMetaFile meta_file;
		if (sidecar_exists)
		{
			if (auto parsed = m_Serializer.DeserializeFromFile<AssetMetaFile>(sidecar_path))
			{
				meta_file = std::move(*parsed);
			}
		}

		// Resolve name
		std::string name = meta_file.Name.empty() ? DefaultNameFromPath(assetFile) : meta_file.Name;

		// If sidecar declares a type, warn about mismatch
		if (!meta_file.Type.empty() && meta_file.Type != loader->TypeName)
		{
			LM_CORE_WARN("AssetServer: '{}' meta says type='{}' but loader for '{}' is '{}'.", assetFile.string(),
			             meta_file.Type, ext, loader->TypeName);
		}
		
		// Generate AssetId
		AssetId id = AssetId::Generate(name);

		AssetMeta meta;
		meta.Id = id;
		meta.Name = name;
		meta.Type = loader->TypeName;
		meta.HotReload = true; // File-based assets support hot-reload by default
		meta.IsDefault = false;

		auto entity = CreateAssetEntity(meta);

		// Type-specific props
		Ref<void> props_blob;
		if (sidecar_exists && !loader->PropsTypeName.empty())
		{
			props_blob = m_Serializer.DeserializeFromFile(loader->PropsTypeName, sidecar_path);
		}

		AssetEntry entry;
		entry.Id = id;
		entry.Entity = entity;
		entry.Loader = loader;
		entry.PrimaryPath = assetFile;
		entry.MetaPath = sidecar_exists ? sidecar_path : std::filesystem::path();
		entry.MetaFile = std::move(meta_file);
		entry.PropsBlob = std::move(props_blob);

		{
			auto lock = WriteLock(m_AssetRegistryMutex);

			if (m_AssetIdsByName.contains(name))
			{
				LM_CORE_WARN("AssetServer: asset name '{}' already registered; skipping '{}'", name, assetFile.string());
				entity.Destruct();
				return;
			}
			if (m_PrimaryByPath.contains(assetFile.string()))
			{
				LM_CORE_WARN("AssetServer: asset path '{}' already registered as primary; skipping", assetFile.string());
				entity.Destruct();
				return;
			}

			m_PrimaryByPath[assetFile.string()] = id;
			m_PathSubscribers[assetFile.string()].push_back(id);
			if (sidecar_exists)
			{
				m_PrimaryByPath[sidecar_path.string()] = id;
				m_PathSubscribers[sidecar_path.string()].push_back(id);
			}
			m_AssetIdsByName[name] = id;
			m_AssetsById[id] = std::move(entry);
		}
	}

	void AssetServer::RegisterFromMeta(const std::filesystem::path& metaFile_in)
	{
		LM_PROFILE_FUNCTION();

		std::error_code ec;
		auto metaFile = std::filesystem::weakly_canonical(metaFile_in, ec);
		if (ec) metaFile = metaFile_in;

		AssetMetaFile meta_file{};
		if (auto parsed = m_Serializer.DeserializeFromFile<AssetMetaFile>(metaFile))
		{
			meta_file = std::move(*parsed);
		}
		else
		{
			LM_CORE_WARN("AssetServer: failed to parse '{}'", metaFile.string());
			return;
		}

		if (meta_file.Type.empty())
		{
			LM_CORE_WARN("AssetServer: '{}' has no `type` field; ignoring", metaFile.string());
			return;
		}

		ErasedLoader* loader;
		{
			auto lock = ReadLock(m_TypeRegistryMutex);

			auto loader_it = m_LoadersByType.find(meta_file.Type);
			if (loader_it == m_LoadersByType.end())
			{
				LM_CORE_WARN("AssetServer: '{}' references unregistered type '{}'", metaFile.string(), meta_file.Type);
				return;
			}
			loader = &loader_it->second;
		}

		// Resolve Name: explicit override or "stripped-meta-suffix" fallback
		std::string name = meta_file.Name;
		if (name.empty())
		{
			auto base = StripAssetLuaSuffix(metaFile);
			auto rel = std::filesystem::relative(base, m_AssetRoot);
			name = rel.generic_string();
		}

		AssetId id = AssetId::Generate(name);

		AssetMeta meta;
		meta.Id = id;
		meta.Name = name;
		meta.Type = loader->TypeName;
		meta.HotReload = true; // File-based assets support hot-reload by default
		meta.IsDefault = false;

		auto entity = CreateAssetEntity(meta);

		Ref<void> props_blob;
		if (!loader->PropsTypeName.empty())
		{
			props_blob = m_Serializer.DeserializeFromFile(loader->PropsTypeName, metaFile);
		}

		AssetEntry entry;
		entry.Id = id;
		entry.Entity = entity;
		entry.Loader = loader;
		entry.PrimaryPath = std::filesystem::path{};
		entry.MetaPath = metaFile;
		entry.MetaFile = std::move(meta_file);
		entry.PropsBlob = std::move(props_blob);

		if (loader->CollectSourcePaths)
		{
			LoaderContext ctx;
			ctx.AssetRoot = m_AssetRoot;
			ctx.Meta = meta;
			ctx.MetaPath = metaFile;
			ctx.Props = entry.PropsBlob;

			std::vector<std::filesystem::path> source_paths;
			loader->CollectSourcePaths(ctx, source_paths);

			entry.AdditionalPaths.reserve(source_paths.size());
			for (auto& src : source_paths)
			{
				std::error_code src_ec;
				auto canonical_src = std::filesystem::weakly_canonical(src, src_ec);
				entry.AdditionalPaths.push_back(src_ec ? src : canonical_src);
			}
		}

		{
			auto lock = WriteLock(m_AssetRegistryMutex);

			if (m_AssetsById.contains(id))
			{
				LM_CORE_ERROR("AssetServer: name '{}' already registered (file: {})", name, metaFile.string());
				entity.Destruct();
				return;
			}
			if (m_PrimaryByPath.contains(metaFile.string()))
			{
				LM_CORE_ERROR("AssetServer: meta path '{}' already registered as primary", metaFile.string());
				entity.Destruct();
				return;
			}

			m_PrimaryByPath[metaFile.string()] = id;
			m_PathSubscribers[metaFile.string()].push_back(id);
			for (const auto& additional : entry.AdditionalPaths)
				m_PathSubscribers[additional.string()].push_back(id);

			m_AssetIdsByName[name] = id;
			m_AssetsById[id] = std::move(entry);
		}
	}
	void AssetServer::RunLoad(AssetEntry& entry)
	{
		LM_PROFILE_FUNCTION();

		if (!entry.Loader)
		{
			LM_CORE_ERROR("Asset '{}' has no loader; cannot load", entry.MetaFile.Name);
			return;
		}

		if (!entry.MetaPath.empty() && !entry.Loader->PropsTypeName.empty())
		{
			entry.PropsBlob = m_Serializer.DeserializeFromFile(entry.Loader->PropsTypeName, entry.MetaPath);
		}

		LoaderContext ctx;
		ctx.AssetRoot = m_AssetRoot;
		ctx.PrimaryPath = entry.PrimaryPath;
		ctx.MetaPath = entry.MetaPath;
		ctx.Props = entry.PropsBlob;
		if (auto meta = entry.Entity.TryGet<AssetMeta>())
		{
			ctx.Meta = *meta;
		}

		if (!entry.Loader->LoadInto(ctx, entry.Entity))
		{
			LM_CORE_ERROR("Failed to load asset '{}'", entry.MetaFile.Name);
			return;
		}
	}
	std::string AssetServer::DefaultNameFromPath(const std::filesystem::path& path) const
	{
		LM_PROFILE_FUNCTION();

		auto rel = std::filesystem::relative(path, m_AssetRoot);
		rel.replace_extension(); // Strip extension
		return rel.generic_string();
	}
	bool AssetServer::IsAssetLuaFile(const std::filesystem::path& path)
	{
		// Check if filename ends with ".asset.lua"
		auto name = path.filename().string();
		constexpr std::string_view suffix = ".asset.lua";
		return name.size() >= suffix.size() && std::equal(suffix.rbegin(), suffix.rend(), name.rbegin());
	}
	std::filesystem::path AssetServer::StripAssetLuaSuffix(const std::filesystem::path& path)
	{
		LM_PROFILE_FUNCTION();
		
		auto stripped = path;
		stripped.replace_extension(); // Remove .lua
		stripped.replace_extension(); // Remove .asset
		return stripped;
	}
}
