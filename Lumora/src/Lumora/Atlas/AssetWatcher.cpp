#include "LMPCH.h"

#include "AssetWatcher.h"

#pragma warning(push, 0)
#include "FileWatch.hpp"
#pragma warning(pop)

namespace Lumora::Atlas
{
	AssetWatcher::AssetWatcher(const std::filesystem::path& root)
	{
		LM_PROFILE_FUNCTION();

		std::error_code ec;
		m_Root = std::filesystem::weakly_canonical(root, ec);
		if (ec)
		{
			LM_CORE_ERROR("AssetWatcher: failed to canonicalise '{}': {}", root.string(), ec.message());
			m_Root = root;
		}

		if (!std::filesystem::exists(m_Root))
		{
			LM_CORE_ERROR("AssetWatcher: root '{}' does not exist", m_Root.string());
			return;
		}

		try
		{
			m_Watcher = CreateScope<Watcher>(m_Root, [this](const std::wstring& relPath, filewatch::Event event)
			{
				OnEvent(std::filesystem::path{relPath}, event); 
			});
		}
		catch (const std::exception& e)
		{
			LM_CORE_ERROR("AssetWatcher: failed to create watcher for '{}': {}", m_Root.string(), e.what());
			m_Watcher.reset();
		}
	}
	AssetWatcher::~AssetWatcher()
	{
		LM_PROFILE_FUNCTION();

		m_Watcher.reset();
	}
	void AssetWatcher::Drain(const std::function<void(const std::filesystem::path&)>& onChangedCallback)
	{
		LM_PROFILE_FUNCTION();

		std::queue<std::filesystem::path> local;
		{
			auto lock = WriteLock(m_QueueMutex);
			local.swap(m_Queue);
		}
		if (local.empty())
			return;

		std::unordered_set<std::string> seen;
		seen.reserve(local.size());

		while (!local.empty())
		{
			auto p = std::move(local.front());
			local.pop();

			auto key = p.string();
			if (!seen.insert(std::move(key)).second)
				continue;

			onChangedCallback(p);
		}
	}
	void AssetWatcher::OnEvent(const std::filesystem::path& relPath, filewatch::Event event)
	{
		std::error_code ec;
		auto absolute = std::filesystem::weakly_canonical(m_Root / relPath, ec);
		if (ec)
			absolute = m_Root / relPath;

		auto lock = WriteLock(m_QueueMutex);
		m_Queue.emplace(std::move(absolute));
	}
}
