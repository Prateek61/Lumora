#include "LMPCH.h"
#include "AssetReloader.h"

namespace
{
	bool IsMetaFile(const std::filesystem::path& path)
	{
		// Check if the file has a .meta.lua extension
		return path.extension() == ".lua" && path.stem().extension() == ".meta";
	}
}

namespace Lumora
{
	namespace Internal
	{
		void AssetReloadQueue::Push(AssetIdT id)
		{
			LM_PROFILE_FUNCTION();

			if (!m_QueuedIds.contains(id))
			{
				m_Queue.push(id);
				m_QueuedIds.insert(id);
			}
		}

		AssetIdT AssetReloadQueue::Pop()
		{
			LM_PROFILE_FUNCTION();

			AssetIdT id = m_Queue.front();
			m_Queue.pop();
			m_QueuedIds.erase(id);

			return id;
		}
	}

	AssetReloader::AssetReloader(std::filesystem::path basePath, std::function<void(AssetIdT id)> reloadCallback, std::function<void(const std::filesystem::path&)> metadataCallback)
		: m_BasePath(std::move(basePath))
		, m_ReloadCallback(std::move(reloadCallback))
		, m_MetadataCallback(std::move(metadataCallback))
	{
	}

	void AssetReloader::WatchFile(const std::filesystem::path& path, AssetIdT id)
	{
		LM_PROFILE_FUNCTION();

		LM_LOCK_READ(m_MapMutex);
		m_PathToIdMap[GetCanonicalPath(path)] = id;
	}

	void AssetReloader::UnwatchFile(const std::filesystem::path& path)
	{
		LM_PROFILE_FUNCTION();

		LM_LOCK_WRITE(m_MapMutex);
		m_PathToIdMap.erase(GetCanonicalPath(path));
	}

	void AssetReloader::UnwatchAsset(AssetIdT id)
	{
		LM_PROFILE_FUNCTION();

		LM_LOCK_WRITE(m_MapMutex);

		for (auto it = m_PathToIdMap.begin(); it != m_PathToIdMap.end(); )
		{
			if (it->second == id)
			{
				it = m_PathToIdMap.erase(it);
			}
			else
			{
				++it;
			}
		}
	}

	void AssetReloader::StartWatching()
	{
		LM_PROFILE_FUNCTION();

		if (m_Watcher.get())
		{
			return;
		}

		auto func = [this](const std::filesystem::path& path, const filewatch::Event changeType)
		{
			OnFileEvent(path, changeType);
		};

		m_Watcher = CreateScope<WatcherType>(m_BasePath.wstring(), func);
	}

	void AssetReloader::StopWatching()
	{
		LM_PROFILE_FUNCTION();

		m_Watcher.reset(nullptr);
	}

	void AssetReloader::RunReloadThread()
	{
		LM_PROFILE_FUNCTION();

		if (m_Running == true)
		{
			return;
		}

		m_Running = true;
		auto fn = [this]()
		{
			while (m_Running)
			{
				// Sleep for 1 ms because VSCode wants to do 2 file events for a single save operation
				std::this_thread::sleep_for(std::chrono::milliseconds(1));

				// Do another check for running
				if (!m_Running)
				{
					break;
				}

				InternalUpdate();
			}
		};
		m_ReloaderThread = std::thread(fn);
	}

	void AssetReloader::StopReloadThread()
	{
		LM_PROFILE_FUNCTION();

		if (m_Running == false)
		{
			return;
		}

		m_Running = false;
		m_CV.notify_one();
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		m_CV.notify_one();

		m_ReloaderThread.join();
	}

	std::filesystem::path AssetReloader::GetCanonicalPath(const std::filesystem::path& path) const
	{
		LM_PROFILE_FUNCTION();

		return weakly_canonical(path).lexically_normal();
	}

	void AssetReloader::OnFileEvent(const std::filesystem::path& path, filewatch::Event changeType)
	{
		LM_PROFILE_FUNCTION();

		auto canonical_path = GetCanonicalPath(path);
		AssetIdT id = g_INVALID_ASSET_ID;

		{
			LM_LOCK_READ(m_MapMutex);
			auto it = m_PathToIdMap.find(canonical_path);
			if (it != m_PathToIdMap.end())
			{
				id = it->second;
			}
		}

		// File is not in the watch list
		if (id == g_INVALID_ASSET_ID)
		{
			if ((changeType == filewatch::Event::added || changeType == filewatch::Event::modified) && IsMetaFile(canonical_path))
			{
				if (m_MetadataCallback)
				{
					m_MetadataCallback(canonical_path);
				}
			}
			return;
		}

		{
			LM_LOCK_WRITE(m_QueueMutex);
			m_ReloadQueue.Push(id);
		}
		// Notify the reload thread
		m_CV.notify_one();
	}

	void AssetReloader::InternalUpdate()
	{
		LM_CORE_ASSERT(false, "Not Implemented")
	}

}
