#pragma once

#include "Lumora/Core/Threading.h"
#include "Lumora/Core/SmartPointers.h"

#include <queue>
#include <filesystem>

namespace filewatch
{
	enum class Event;
	template <class StringType>
	class FileWatch;
}

namespace Lumora::Atlas
{
	class AssetWatcher
	{
	public:
		explicit AssetWatcher(const std::filesystem::path& root);
		~AssetWatcher();

		AssetWatcher(const AssetWatcher&) = delete;
		AssetWatcher& operator=(const AssetWatcher&) = delete;

		void Drain(const std::function<void(const std::filesystem::path&)>& onChangedCallback);

		const std::filesystem::path& GetRoot() const { return m_Root; }

	private:
		using Watcher = filewatch::FileWatch<std::wstring>;

		void OnEvent(const std::filesystem::path& relPath, filewatch::Event event);

		Scope<Watcher> m_Watcher;
		std::filesystem::path m_Root;
		Mutex m_QueueMutex;
		std::queue<std::filesystem::path> m_Queue;
	};
}
