#pragma once

#include "Lumora/Atlas/AssetCommon.h"
#include "Lumora/Atlas/AssetLoader.h"
#include "Lumora/Aether/World.h"
#include "Lumora/Core/Assert.h"
#include "Lumora/Utilities/TSQueue.h"

namespace Lumora::Rune
{
	class LuaSerializer;
}

namespace Lumora::Atlas
{
	class AssetServer
	{
	public:
		AssetServer(Aether::World& world, Rune::LuaSerializer& serializer);
		~AssetServer();

		AssetServer(const AssetServer&) = delete;
		AssetServer& operator=(const AssetServer&) = delete;

		// Loader Registration
		template <typename T, typename Decoded = T>
		void RegisterLoader(AssetLoader<T, Decoded> loader);

		// Runtime (memory-only) insertion
		template <typename T>
		AssetHandle<T> Add(T&& value, std::string name);

		// Defaults
		template <typename T>
		void SetDefault(T&& value);
		template <typename T>
		AssetHandle<T> GetDefault() const;

		// Lookup
		template <typename T>
		AssetHandle<T> Get(std::string_view name) const;
		UntypedAssetHandle GetUntyped(std::string_view name) const;
		Aether::Entity Resolve(AssetId id) const;
		Aether::Entity Resolve(std::string_view name) const;

		// Unload
		bool Unload(AssetId id);

		// Discovery
		void Scan(const std::filesystem::path& assetRoot);

		// For one-off loading
		template <typename T>
		AssetHandle<T> Load(const std::filesystem::path& path);

		// Reload
		void QueueReload(AssetId id);
		void Pump();

		std::vector<AssetId> LookupByPath(const std::filesystem::path& canonicalPath) const;

		const std::filesystem::path& GetAssetRoot() const { return m_AssetRoot; }

	private:
		struct AssetEntry
		{
			AssetId Id;
			Aether::Entity Entity;
			const ErasedLoader* Loader = nullptr;
			std::filesystem::path PrimaryPath;
			std::filesystem::path MetaPath;
			std::vector<std::filesystem::path> AdditionalPaths;
			AssetMetaFile MetaFile;
			Ref<void> PropsBlob;
		};

		Aether::World& m_World;
		Rune::LuaSerializer& m_Serializer;
		std::filesystem::path m_AssetRoot;

		Aether::Entity m_AssetsRootEntity; // Parent for all asset entities, used for organization

		// Loader Registry
		std::unordered_map<std::string, ErasedLoader> m_LoadersByType;
		std::unordered_map<std::string, std::string> m_TypeNameByExtension;

		// Asset Registry
		std::unordered_map<AssetId, AssetEntry> m_AssetsById;
		std::unordered_map<std::string, AssetId> m_AssetIdsByName;
		std::unordered_map<std::string, AssetId> m_PrimaryByPath;
		std::unordered_map<std::string, std::vector<AssetId>> m_PathSubscribers;

		// Defaults
		std::unordered_map<std::type_index, AssetId> m_DefaultAssets;

		// Locks
		mutable RWMutex m_TypeRegistryMutex;
		mutable RWMutex m_AssetRegistryMutex;

		// Reload Queue
		Mutex m_ReloadQueueMutex;
		std::vector<AssetId> m_ReloadQueue;

		Aether::Entity CreateAssetEntity(const AssetMeta& meta);

		// Internal helpers
		void RegisterSingleFile(const std::filesystem::path& assetFile);
		void RegisterFromMeta(const std::filesystem::path& metaFile);
		void RunLoad(AssetEntry& entry);

		std::string DefaultNameFromPath(const std::filesystem::path& path) const;

		// Naming convention helpers
		static bool IsAssetLuaFile(const std::filesystem::path& path);
		static std::filesystem::path StripAssetLuaSuffix(const std::filesystem::path& path);
	};
}

// Template Implementation
namespace Lumora::Atlas
{
	template <typename T, typename Decoded>
	void AssetServer::RegisterLoader(AssetLoader<T, Decoded> loader)
	{
		LM_PROFILE_FUNCTION();

		LM_CORE_ASSERT(!loader.TypeName.empty(), "AssetLoader must have a TypeName.");
		LM_CORE_ASSERT(loader.Decode, "AssetLoader::Decode must be set");
		LM_CORE_ASSERT(loader.Finalize, "AssetLoader::Finalize must be set (only T==Decoded gets a default passthrough)");

		auto erased = ErasedLoader::From(std::move(loader));

		auto lock = WriteLock(m_TypeRegistryMutex);

		// TypeName collision check
		if (m_LoadersByType.contains(erased.TypeName))
		{
			LM_CORE_ERROR("AssetServer: loader type '{}' already registered; Skipping", erased.TypeName);
			return;
		}

		// Extension
		for (const auto& ext : erased.FileExtensions)
		{
			// Extension collision check
			auto it = m_TypeNameByExtension.find(ext);
			if (it != m_TypeNameByExtension.end())
			{
				LM_CORE_ERROR("AssetServer: extension '{}' already claimed by '{}'; skipping registration of '{}'", ext, it->second,
				              erased.TypeName);
				return;
			}

			m_TypeNameByExtension[ext] = erased.TypeName;
		}

		m_LoadersByType[erased.TypeName] = std::move(erased);
	}

	template <typename T>
	AssetHandle<T> AssetServer::Add(T&& value, std::string name)
	{
		LM_PROFILE_FUNCTION();

		AssetId id = AssetId::Generate(name);
		LM_CORE_ASSERT(id.IsValid(), "Generated AssetId is invalid. Name may be empty or too long.");

		AssetMeta meta;
		meta.Id = id;
		meta.Name = std::move(name);
		meta.Type = typeid(T).name();
		meta.HotReload = false; // Memory-only assets don't support hot-reload
		meta.IsDefault = false;

		auto lock = WriteLock(m_AssetRegistryMutex);

		if (m_AssetIdsByName.contains(meta.Name))
		{
			LM_CORE_WARN("AssetServer::Add: name '{}' already registered", name);
			return {};
		}

		auto entity = CreateAssetEntity(meta);
		entity.Set(std::forward<T>(value));

		AssetEntry entry;
		entry.Id = id;
		entry.Entity = entity;

		m_AssetsById[id] = std::move(entry);
		m_AssetIdsByName[meta.Name] = id;

		return AssetHandle<T>{id, entity};
	}

	template <typename T>
	void AssetServer::SetDefault(T&& value)
	{
		LM_PROFILE_FUNCTION();

		// 1. If a default already exists for T, unload it first
		AssetId existing = AssetId::Invalid();
		{
			auto lock = WriteLock(m_AssetRegistryMutex);
			auto it = m_DefaultAssets.find(typeid(T));
			if (it != m_DefaultAssets.end())
			{
				existing = it->second;
			}
		}
		if (existing.IsValid())
		{
			Unload(existing);
		}

		// 2. Add the new value under the type's reserved internal name.
		std::string internal_name = std::string("__default__") + typeid(T).name();
		auto handle = Add(std::forward<T>(value), internal_name);
		if (!handle.IsValid())
		{
			LM_CORE_ERROR("Failed to set default asset for type '{}'", typeid(T).name());
			return;
		}

		// 3. Mark as the default and index in m_Defaults
		auto lock = WriteLock(m_AssetRegistryMutex);
		m_DefaultAssets[typeid(T)] = handle.Id;

		Aether::Entity e = handle.Entity;
		if (auto* meta = e.TryGetMut<AssetMeta>())
			meta->IsDefault = true;
	}

	template <typename T>
	AssetHandle<T> AssetServer::GetDefault() const
	{
		LM_PROFILE_FUNCTION();

		auto lock = ReadLock(m_AssetRegistryMutex);

		auto it = m_DefaultAssets.find(typeid(T));
		if (it == m_DefaultAssets.end())
		{
			return {};
		}

		auto entry_it = m_AssetsById.find(it->second);
		if (entry_it == m_AssetsById.end())
		{
			LM_CORE_ERROR("Default asset with id {} found in m_Defaults but missing from m_AssetsById", it->second.Id);
			return {};
		}

		return AssetHandle<T>{entry_it->second.Id, entry_it->second.Entity};
	}

	template <typename T>
	AssetHandle<T> AssetServer::Get(std::string_view name) const
	{
		LM_PROFILE_FUNCTION();

		auto lock = ReadLock(m_AssetRegistryMutex);

		auto name_it = m_AssetIdsByName.find(std::string(name));
		if (name_it != m_AssetIdsByName.end())
		{
			const auto& entry = m_AssetsById.at(name_it->second);
			return AssetHandle<T>{entry.Id, entry.Entity};
		}

		// Fallback to default for type T.
		LM_CORE_TRACE("Asset '{}' not found; falling back to default for type '{}'", name, typeid(T).name());

		auto default_it = m_DefaultAssets.find(typeid(T));
		if (default_it != m_DefaultAssets.end())
		{
			const auto& entry = m_AssetsById.at(default_it->second);
			return AssetHandle<T>{entry.Id, entry.Entity};
		}

		return {};
	}

	template <typename T>
	AssetHandle<T> AssetServer::Load(const std::filesystem::path& path)
	{
		LM_PROFILE_FUNCTION();

		auto canonical = std::filesystem::weakly_canonical(path);

		// Already registered? Return existing handle.
		{
			auto lock = ReadLock(m_AssetRegistryMutex);
			auto id_it = m_PrimaryByPath.find(canonical.string());
			if (id_it != m_PrimaryByPath.end())
			{
				const auto& entry = m_AssetsById.at(id_it->second);
				return AssetHandle<T>{entry.Id, entry.Entity};
			}
		}

		// Dispatch by file kind
		if (IsAssetLuaFile(canonical))
		{
			auto primary = StripAssetLuaSuffix(canonical);
			if (std::filesystem::exists(primary))
			{
				RegisterSingleFile(primary);
			}
			else
			{
				RegisterFromMeta(canonical);
			}
		}
		else
		{
			RegisterSingleFile(canonical);
		}

		// Run Loader
		AssetEntry snapshot;
		{
			auto lock = ReadLock(m_AssetRegistryMutex);
			auto path_it = m_PrimaryByPath.find(canonical.string());
			if (path_it == m_PrimaryByPath.end())
			{
				LM_CORE_ERROR("Failed to load asset from '{}': registration failed", canonical.string());
				return {};
			}

			auto entry_it = m_AssetsById.find(path_it->second);
			if (entry_it == m_AssetsById.end())
			{
				LM_CORE_ERROR("Failed to load asset from '{}': registered id {} not found in m_AssetsById", canonical.string(),
				              path_it->second.Id);
				return {};
			}
			snapshot = entry_it->second;
		}

		RunLoad(snapshot);

		{
			auto lock = WriteLock(m_AssetRegistryMutex);
			auto it = m_AssetsById.find(snapshot.Id);
			if (it != m_AssetsById.end())
				it->second.PropsBlob = snapshot.PropsBlob;
		}

		return AssetHandle<T>{snapshot.Id, snapshot.Entity};
	}
}
