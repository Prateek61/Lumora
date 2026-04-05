#pragma once

#include <vector>

namespace Lumora::Core
{
	template <typename T>
	struct Events
	{
		void Send(const T& event) { m_Buffer.push_back(event); }
		void Send(T&& event) { m_Buffer.push_back(std::move(event)); }
		void Clear() { m_Buffer.clear(); }
		bool Empty() const { return m_Buffer.empty(); }
		size_t Size() const { return m_Buffer.size(); }

		// Range-based for loop support
		auto begin() { return m_Buffer.begin(); }
		auto end() { return m_Buffer.end(); }

	private:
		std::vector<T> m_Buffer;
	};
}