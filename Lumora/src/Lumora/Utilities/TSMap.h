#pragma once

#include "Lumora/Core/Threading.h"

#include <map>
#include <optional>

namespace Lumora
{
	template<typename K, typename V>
	class TSMap
	{
	public:
		TSMap() = default;
		~TSMap() = default;

		/// Raw access for bulk work. Hold GetMutex() yourself while you use it.
		std::map<K, V>& GetMap();
		RWMutex& GetMutex();

		void Set(K key, V value);
		/// Returns a copy: the lock is gone by the time you read it.
		std::optional<V> Get(const K& key);
		bool Remove(const K& key);

	private:
		std::map<K, V> m_Map;
		RWMutex m_Mutex;
	};
}

// Implementation
namespace Lumora
{
	template<typename K, typename V>
	std::map<K, V>& TSMap<K, V>::GetMap()
	{
		return m_Map;
	}

	template<typename K, typename V>
	RWMutex& TSMap<K, V>::GetMutex()
	{
		return m_Mutex;
	}

	template<typename K, typename V>
	void TSMap<K, V>::Set(K key, V value)
	{
		auto lock = WriteLock<RWMutex>(m_Mutex);

		m_Map[std::move(key)] = std::move(value);
	}

	template<typename K, typename V>
	std::optional<V> TSMap<K, V>::Get(const K& key)
	{
		auto lock = ReadLock<RWMutex>(m_Mutex);
		auto it = m_Map.find(key);
		if (it != m_Map.end())
		{
			return it->second;
		}
		return std::nullopt;
	}

	template<typename K, typename V>
	bool TSMap<K, V>::Remove(const K& key)
	{
		auto lock = WriteLock<RWMutex>(m_Mutex);

		return m_Map.erase(key) > 0;
	}
}
