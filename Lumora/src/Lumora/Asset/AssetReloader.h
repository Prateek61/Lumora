#pragma once

#include "Lumora/Common/Threading.h"
#include "Lumora/Asset/AssetCommon.h"
#include <queue>
#include <set>

// Suppress warnings in filewatch library
#pragma warning(push, 0)
#include "FileWatch.hpp"
#pragma warning(pop)

namespace Lumora
{
	namespace Internal
	{
		class AssetReloadQueue
		{
		public:
			void Push(AssetIdT id);
			AssetIdT Pop();
			AssetIdT Peek() const { return m_Queue.front(); }

			size_t Size() const { return m_Queue.size(); }
			bool Empty() const { return m_Queue.empty(); }

		private:
			std::queue<AssetIdT> m_Queue;
			std::set<AssetIdT> m_QueuedIds;
		};
	}

	class AssetReloader
	{
	public:
		AssetReloader(std::filesystem::path basePath, std::function<void(AssetIdT id)> reloadCallback, std::function<void(const std::filesystem::path&)> metadataCallback);
		~AssetReloader();

		void WatchFile(const std::filesystem::path& path, AssetIdT id);
		void UnwatchFile(const std::filesystem::path& path);
		void UnwatchAsset(AssetIdT id);

		void StartWatching();
		void StopWatching();

		void RunReloadThread();
		void StopReloadThread();
	private:
		std::filesystem::path m_BasePath;
		std::function<void(AssetIdT id)> m_ReloadCallback;
		std::function<void(const std::filesystem::path&)> m_MetadataCallback;

		using WatcherType = filewatch::FileWatch<std::wstring>;
		Scope<WatcherType> m_Watcher;

		std::atomic_bool m_Running = false;
		std::thread m_ReloaderThread;

		AssetMap<std::filesystem::path, AssetIdT> m_PathToIdMap;
		Internal::AssetReloadQueue m_ReloadQueue;

		RWMutex m_MapMutex;
		Mutex m_QueueMutex;
		ConditionVariable m_CV;

	private:
		std::filesystem::path GetCanonicalPath(const std::filesystem::path& path) const;
		void OnFileEvent(const std::filesystem::path& path, filewatch::Event changeType);
		void InternalUpdate();
	};
}