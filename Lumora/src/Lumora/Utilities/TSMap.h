#pragma once

#include "Lumora/Common/Threading.h"

namespace Lumora
{
	template<typename K, typename V>
	class TSMap
	{
	public:
		TSMap() = default;
		~TSMap() = default;

		std::map<K, V>& GetMap();
		RWMutex& GetMutex();

		void Set(K key, V value);
		std::optional<V&> Get(const K& key);
		bool Remove(K key);

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

		m_Map[key] = value;
	}

	template<typename K, typename V>
	std::optional<V&> TSMap<K, V>::Get(const K& key)
	{
		auto lock = ReadLock<RWMutex>(m_Mutex);
		auto it = m_Map.find(key);
		if (it != m_Map.end())
		{
			return it->second;
		}
		return std::nullopt;
	}
}