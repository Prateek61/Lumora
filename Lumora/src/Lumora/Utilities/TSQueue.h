#pragma once

#include "Lumora/Core/Threading.h"
#include <optional>
#include <queue>

namespace Lumora
{
	template <typename T>
	class TSQueue
	{
	public:
		TSQueue() = default;
		~TSQueue() = default;

		std::size_t Size();
		bool Empty();
		/// By value, so it takes lvalues and rvalues alike and moves either one in.
		void Push(T item);
		std::optional<T> TryPop();
		std::optional<T> PopBlocking();
		void Shutdown();

	private:
		mutable Mutex m_Mutex;
		ConditionVariable m_CV;
		std::queue<T> m_Queue;
		bool m_Shutdown = false;
	};
}

// Implementation
namespace Lumora
{
	template <typename T>
	std::size_t TSQueue<T>::Size()
	{
		auto lock = WriteLock(m_Mutex);
		return m_Queue.size();
	}

	template <typename T>
	bool TSQueue<T>::Empty()
	{
		return Size() == 0;
	}

	template <typename T>
	void TSQueue<T>::Push(T item)
	{
		auto lock = WriteLock(m_Mutex);
		m_Queue.push(std::move(item));
		m_CV.notify_one();
	}

	template <typename T>
	std::optional<T> TSQueue<T>::TryPop()
	{
		auto lock = WriteLock(m_Mutex);
		if (m_Queue.empty())
			return std::nullopt;
		T item = std::move(m_Queue.front());
		m_Queue.pop();
		return item;
	}

	template <typename T>
	std::optional<T> TSQueue<T>::PopBlocking()
	{
		auto lock = WriteLock(m_Mutex);
		m_CV.wait(lock, [this] { return !m_Queue.empty() || m_Shutdown; });
		if (m_Queue.empty())
			return std::nullopt; // Queue was shutdown
		T item = std::move(m_Queue.front());
		m_Queue.pop();
		return item;
	}

	template <typename T>
	void TSQueue<T>::Shutdown()
	{
		auto lock = WriteLock(m_Mutex);
		m_Shutdown = true;
		m_CV.notify_all();
	}
}
